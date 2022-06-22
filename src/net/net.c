#include <stdio.h>
#include "global.h"
#include "prt.h"
#include "net.h"
#include "uart.h"
#include "common_var.h"
#include <errno.h>
#include "cJSON.h"
#include "curl/curl.h"



unsigned char g_upload_flag = 0;
unsigned char s_exit_flag = 0;
//static const char *s_user_name = "admin";
//static const char *s_password = "password";
static const char *s_user_name = "TaxPrinter";
static const char *s_password = "F34500C275E5A4AA";
//static const char *pub_topic_heartbeat = "/smart9/up/heartbeat/";
//static const char *sub_topic_heartbeat = "/smart9/down/heartbeat/1122334455667788";
static const char *pub_topic_upload = "/smart9/up/upload/1122334455667788";
static const char *sub_topic_upload = "/smart9/down/upload/1122334455667788";
static const char *sub_topic_d9 = "UPLOADEX";
static const char *pub_topic_heartbeat = "/smart9_status_up";
char sub_topic_heartbeat[64] = {0};
char sub_topic_prt[64] = {0};
char sub_topic_op[64] = {0};
static struct mg_mqtt_topic_expression s_topic_expr = {NULL, 0};

extern char* GF_GetGUID(char * buf);

uint32_t tcp_init(const char *port)
{
    pn_data.len = 0;
    pn_data.is_handle = 0;
    pn_data_buf.len = 0;
    mg_mgr_init(&m_tcp, NULL);
    mg_bind(&m_tcp, port, tcp_handler);
    printf("Starting tcp mgr on port %s\n", port);
    return D9_OK;
}

bool search_key_words(unsigned char *ptr, int len)
{
    int offset = 0, i = 0;
    bool found = false;
    char *lowers = (char *)malloc(len);
    //printf("dbgggg1\n");
    for (i = 0; i < len; i++)
    {
        lowers[i] = tolower(ptr[i]);
        ///printf("%c", lowers[i]);
    }
   // printf("dbgggg3, len is %d\n", len);
    for (offset = 0; offset < len; offset++)
    {
        printf("%c", ptr[offset]);
        // if(memcmp(lowers + offset, "copy", strlen("copy")) == 0) 
        // {
        //     printf("copy found, not a receipt\n");
        //     goto end;
        // }
        //printf("offset = %d, g_key_words_count = %d\n", offset, g_key_words_count);
        for(i = 0; i < g_key_words_count; i++) 
        {
            if (offset + g_key_words_lengths[i] <= len)
            {
                if (memcmp(lowers + offset, g_key_words[i], g_key_words_lengths[i]) == 0)
                {
                    found = true;
                    goto end;
                } 
            }
        }
    }

end:
    printf("search_key_words return: %d\n", found);
    if (lowers)
    {
        free(lowers);
    }
    return found;
}

bool has_copy(unsigned char *ptr, int len)
{
    bool found = false;
    int i = 0;
    for (i = 0; i < len - strlen("Copy"); i++)
    {
        if(memcmp(ptr + i, "Copy", strlen("Copy")) == 0) 
        {
            printf("copy found, not a formal receipt\n");
            return true;
        }
    }
    return false;
}

void process_data(pdata_node node)
{
    int j = 0;
    unsigned char ctrl_upload_flag = 0;
    FILE *fp;
    char ret_buff[512] = {0};
    printf("process_data1! node->len is %d\n", node->len);
    node->is_receipt = ctrl_upload_flag = search_key_words(node->data, node->len);
    if (node->is_receipt && has_copy(node->data, node->len))
    {
        node->is_copy = true;
        ctrl_upload_flag = false;
    }
    printf("is_receipt = %d，ctrl_upload_flag = %d, is_copy = %d\n", node->is_receipt, ctrl_upload_flag, node->is_copy);
    
    // if (search_key_words(node->data, node->len))
    // {
    //     ctrl_upload_flag = 1;
    //     node->is_receipt = 1;
    // }
    
    // for(j = 0; j < node->len; j++)
    // {   
    //     printf("%c", node->data[j]);
    //     if(strncmp(&node->data[j], "Scan Kode Sid9", strlen("Scan Kode Sid9")) == 0)
    //     {
    //         printf("need printf1!\n");
    //         ctrl_upload_flag = 1;
    //         node->is_receipt = 1;
    //         break;
    //     }
    // }
    if(ctrl_upload_flag == 1)
    {
        ctrl_upload_flag = 0;
        fp = popen("rm ./escode/code.bin", "r");
        if(fp != NULL)
        {
            while(fgets(ret_buff, sizeof(ret_buff), fp) != NULL)
            {
                if('\n' == ret_buff[strlen(ret_buff)-1])
                {
                    ret_buff[strlen(ret_buff)-1] = '\0';
                }
                printf("rm ./escode/code.bin = %s\r\n", ret_buff);
            }
            pclose(fp);
        }
        else
        {
            printf("popen faild!!!!!!!!!!\n");
        }
        //system("rm ./escode/code.bin");
        dump_data("./escode/code.bin", node->data, node->len);
        fp = popen("rm ./escode/upload.zip", "r");
        if(fp != NULL)
        {
            while(fgets(ret_buff, sizeof(ret_buff), fp) != NULL)
            {
                if('\n' == ret_buff[strlen(ret_buff)-1])
                {
                    ret_buff[strlen(ret_buff)-1] = '\0';
                }
                printf("rm ./escode/upload.zip = %s\r\n", ret_buff);
            }
            pclose(fp);
        }
        else
        {
            printf("popen faild!!!!!!!!!!\n");
        }
        fp = popen("zip -r ./escode/upload.zip ./escode/*", "r");
        if(fp != NULL)
        {
            while(fgets(ret_buff, sizeof(ret_buff), fp) != NULL)
            {
                if('\n' == ret_buff[strlen(ret_buff)-1])
                {
                    ret_buff[strlen(ret_buff)-1] = '\0';
                }
                printf("zip = %s\r\n", ret_buff);
            }
            pclose(fp);
        }
        else
        {
            printf("popen faild!!!!!!!!!!\n");
        }            
        g_upload_flag = 1;  
    }
    else
    {
        g_printing_flag = true;
        printf("print 2222, node->len=%d\n", node->len);
        if (node->is_copy)
        {
            prt_handle.esc_2_prt(node->data, node->len);
            prt_handle.printer_cut(96);
        }
        else
        {
            prt_handle.esc_2_prt(node->data, node->len + strlen(ESCPOS_CMD_CUT1));
        }
        
        //prt_handle.printer_cut(96);
        //prt_handle.esc_2_prt(ESCPOS_CMD_INIT, strlen(ESCPOS_CMD_INIT));
        //prt_handle.esc_2_prt(ESCPOS_CMD_CUT1, strlen(ESCPOS_CMD_CUT1));
        g_printing_flag = false;
        node->len = 0;
        destroy_node(node);
        printf("only prt end 1, prt data 0\n");                    
    }  
}

void process_incoming_data(pdata_node prt_data)
{
    FILE *fp;
    // (void)p;
    char ret_buff[512] = {0};

    if(prt_data->data[prt_data->len - 1] == 0x69 && prt_data->data[prt_data->len - 2] == 0x1b)
    {
        printf("start combine data2!\n");
        prt_data->len -= 2;
        
        fp = popen("rm ./escode/code.bin", "r");
        if(fp != NULL)
        {
            while(fgets(ret_buff, sizeof(ret_buff), fp) != NULL)
            {
                if('\n' == ret_buff[strlen(ret_buff)-1])
                {
                    ret_buff[strlen(ret_buff)-1] = '\0';
                }
                printf("rm ./escode/code.bin = %s\r\n", ret_buff);
            }
            pclose(fp);
        }
        else
        {
            printf("popen faild!!!!!!!!!!\n");
        }
        //system("rm ./escode/code.bin");
        dump_data("./escode/code.bin", prt_data->data, prt_data->len);
        fp = popen("rm ./escode/upload.zip", "r");
        if(fp != NULL)
        {
            while(fgets(ret_buff, sizeof(ret_buff), fp) != NULL)
            {
                if('\n' == ret_buff[strlen(ret_buff)-1])
                {
                    ret_buff[strlen(ret_buff)-1] = '\0';
                }
                printf("rm ./escode/upload.zip = %s\r\n", ret_buff);
            }
            pclose(fp);
        }
        else
        {
            printf("popen faild!!!!!!!!!!\n");
        }
        fp = popen("zip -r ./escode/upload.zip ./escode/*", "r");
        if(fp != NULL)
        {
            while(fgets(ret_buff, sizeof(ret_buff), fp) != NULL)
            {
                if('\n' == ret_buff[strlen(ret_buff)-1])
                {
                    ret_buff[strlen(ret_buff)-1] = '\0';
                }
                printf("zip = %s\r\n", ret_buff);
            }
            pclose(fp);
        }
        else
        {
            printf("popen faild!!!!!!!!!!\n");
        }            
        g_upload_flag = 1;            
    }
    printf("debug 102, prt_data.len - 3 = %d\n", prt_data->len - 3);

    //if(prt_data.data[len - 3] == 0x1d && prt_data.data[len - 2] == 0x56 && prt_data.data[len - 1] == 0x01)
    if((memcmp(&prt_data->data[prt_data->len - 3], ESCPOS_CMD_CUT0, strlen(ESCPOS_CMD_CUT0)) == 0) 
    || (memcmp(&prt_data->data[prt_data->len - 3], ESCPOS_CMD_CUT1, strlen(ESCPOS_CMD_CUT1)) == 0) 
    || (memcmp(&prt_data->data[prt_data->len - 3], ESCPOS_CMD_CUT2, strlen(ESCPOS_CMD_CUT2)) == 0))
    {
        printf("debug 103\n");
        // prt_handle.esc_2_prt(prt_data.data, (len - 3));
        // prt_handle.printer_cut(96);
        // len = 0;
        //prt_data->len -= strlen(ESCPOS_CMD_CUT0);
        process_data(prt_data);
        printf("only prt end 2\n");               
    }
    else
    {
        //if(prt_data.data[len - 4] == 0x70 && prt_data.data[len - 5] == 0x1b)
        printf("debug 111\n");
        if ((prt_data->len) < 8) {
            return;
        }
        printf("debug 1111 0x%02X 0x%02X\n", prt_data->data[prt_data->len - 5], prt_data->data[prt_data->len - 4]);
        if(memcmp(&prt_data->data[prt_data->len - 5], ESCPOS_CMD_CASHBOX, strlen(ESCPOS_CMD_CASHBOX)) == 0)
        {
            printf("start combine data!\n");
            memcpy(&prt_data->data[prt_data->len- 8], &prt_data->data[prt_data->len - 5], 5);
            prt_data->len -= 3;
            process_data(prt_data);               
        }
    }
}

void tcp_handler(struct mg_connection *nc, int ev, void *p)
{
    //int i, j = 0;
    //unsigned char ctrl_upload_flag = 0;
    struct mbuf *io = &nc->recv_mbuf;
    static int tcp_rec_len = 0;
    prt_net_data *prt;

    switch (ev)
    {
    case MG_EV_RECV:
        //mg_send(nc, io->buf, io->len); // Echo message back
        // if (g_waiting_online_code_flag)
        // {
             prt = &pn_data_buf;
        // }
        // else 
        // {
        //     prt = &pn_data;
        // }
        printf("get through socket len = %zd, current tcp_rec_len is %d, current prt->len is %d, waiting code flag is %d\n", io->len, tcp_rec_len, prt->len, g_waiting_online_code_flag);
        print_array(io->buf,io->len);
        memcpy(prt->data + (prt->len), io->buf, io->len);
        
        //tcp_rec_len = io->len;
        prt->len += io->len;
        mbuf_remove(io, io->len); // Discard message from recv buffer
        printf("debug 101\n");
        //process loop
        if (memcmp(ESCPOS_CMD_CUT_0, prt->data + prt->len - 2, strlen(ESCPOS_CMD_CUT_0)) == 0)
        {
            create_node(prt->data, prt->len);
            prt->len = 0;
        }
        else
        {
            if((memcmp(prt->data + prt->len - 3, ESCPOS_CMD_CUT1, strlen(ESCPOS_CMD_CUT1)) == 0))
            {
                create_node(prt->data, prt->len);
                prt->len = 0;      
            }
            else
            {
                //if(prt_data.data[len - 4] == 0x70 && prt_data.data[len - 5] == 0x1b)
                printf("debug 1112\n");
                if ((prt->len) < 8) {
                    break;
                }
                
                if(memcmp(prt->data + prt->len - 5, ESCPOS_CMD_CASHBOX, strlen(ESCPOS_CMD_CASHBOX)) == 0)
                {
                    create_node(prt->data, prt->len);
                    prt->len = 0;    
                }
            }
        }
        
        // if (g_waiting_online_code_flag) 
        // {
        //     break;
        // }
        // else
        // {
        //     process_incoming_data(prt, io->len);
        // }
        tcp_rec_len = 0;
        break;
    default:
        break;
    }
}

void tcp_poll(uint32_t i_time)
{
    mg_mgr_poll(&m_tcp, i_time);
}

uint32_t mqtt_state = 0;

uint32_t mqtt_init(const char* address)
{
    mg_mgr_init(&m_mqtt, NULL);
    if (mg_connect(&m_mqtt, address, mqtt_handler) == NULL) {
        fprintf(stderr, "mg_connect(%s) failed\n", address);
        exit(EXIT_FAILURE);
    }
    mqtt_state = 0;

}

uint32_t mqtt_free(struct mg_mgr *m)
{
    mg_mgr_free(m);
    //mg_close_conn(m->active_connections);

}



unsigned char test_feed[3] = {0x1b, 0x64, 0x06};
unsigned char test_cut[4] = {0x1d, 0x56, 0x42, 0x40};

void mqtt_handler(struct mg_connection *nc, int ev, void *p)
{
    cJSON *json = NULL;
    cJSON *code = NULL;
    char prt_sn[32] = {0};
    int de_data_len = 0;
    struct mg_mqtt_message *msg = (struct mg_mqtt_message *)p;
    (void)nc;

    if (ev != MG_EV_POLL) printf("mqtt GOT event %d\n", ev);

    switch (ev) {
    case MG_EV_CONNECT: {
        struct mg_send_mqtt_handshake_opts opts;
        memset(&opts, 0, sizeof(opts));
        opts.user_name = s_user_name;
        opts.password = s_password;
        opts.keep_alive = 10;

        mg_set_protocol_mqtt(nc);
        memset(prt_sn, 0, sizeof(prt_sn));
        prt_handle.get_printer_sn(prt_sn, 32);
        printf("mqtt get_printer_sn %s\n", prt_sn);
        mg_send_mqtt_handshake_opt(nc, prt_sn, opts);
        break;
    }
    case MG_EV_MQTT_CONNACK:
        if (msg->connack_ret_code != MG_EV_MQTT_CONNACK_ACCEPTED) {
            printf("Got mqtt connection error: %d\n", msg->connack_ret_code);
            exit(1);
        }

        memset(prt_sn, 0, sizeof(prt_sn));
        prt_handle.get_printer_sn(prt_sn, 32);
        // s_topic_expr.topic = sub_topic_d9;
        // printf("Subscribing to '%s'\n", sub_topic_d9);
        // mg_mqtt_subscribe(nc, &s_topic_expr, 1, 42);
        s_topic_expr.topic = prt_sn;
        strcpy(sub_topic_prt, prt_sn);
        printf("Subscribing to '%s'\n", prt_sn);
        mg_mqtt_subscribe(nc, &s_topic_expr, 1, 42);
        // s_topic_expr.topic = pub_topic_heartbeat;
        // printf("Subscribing to '%s'\n", pub_topic_heartbeat);
        // mg_mqtt_subscribe(nc, &s_topic_expr, 1, 42);

        strcpy(sub_topic_heartbeat, prt_sn);
        strcpy(&sub_topic_heartbeat[strlen(sub_topic_heartbeat)], "/smart9_status_down");
        s_topic_expr.topic = sub_topic_heartbeat;
        printf("Subscribing to '%s'\n", sub_topic_heartbeat);
        mg_mqtt_subscribe(nc, &s_topic_expr, 1, 42);

        strcpy(sub_topic_op, prt_sn);
        strcpy(&sub_topic_op[strlen(sub_topic_op)], "/smart9_opt");        
        s_topic_expr.topic = sub_topic_op;
        printf("Subscribing to '%s'\n", sub_topic_op);
        mg_mqtt_subscribe(nc, &s_topic_expr, 1, 42);
    
        g_unprint_flag = 0;
        g_offline_flag = 0;
        break;
    case MG_EV_MQTT_PUBACK:
        printf("Message publishing acknowledged (msg_id: %d)\n", msg->message_id);
        break;
    case MG_EV_MQTT_SUBACK:
        printf("Subscription acknowledged\n");
        break;
    case MG_EV_MQTT_PUBLISH: {
        //printf("Got incoming message %.*s: %d\n", (int)msg->topic.len,
        printf("Got incoming message %d - %s: %d\n", (int)msg->topic.len,
            msg->topic.p, (int)msg->payload.len);
        //print_array((unsigned char*)msg->payload.p,(int)msg->payload.len);
        if(strlen(sub_topic_prt) == msg->topic.len)
        {
            printf("publish 1\n");
            if(memcmp(msg->topic.p, sub_topic_prt, strlen(sub_topic_prt)) == 0)
            {
                printf("publish 2\n");
            //dump_data("./prt.bin", msg->payload.p,(int)msg->payload.len);
            //prt_print(pn_data.data, pn_data.len);
            //escpos_printer_feed(3);
            if(g_overtime_flag == 0)
            {
                struct timeval tv;
                gettimeofday (&tv, NULL);
                printf("got online code time = %ld.%ld\n", tv.tv_sec, tv.tv_usec);
                printf("publish 3\n");
                json = cJSON_Parse(msg->payload.p);
                if(!json)
                {
                    printf("ERROR before: [%s]\n", cJSON_GetErrorPtr());
                }     
                code = cJSON_GetObjectItem(json, "id");
                if(code != NULL)
                {
                    if(strcmp(code->valuestring, prt_list->id) != 0)
                    {
                        printf("err id is: %s\n", code->valuestring);
                        cJSON_free(json);
                        cJSON_free(code);
                        return;
                    }                   
                }
                else
                {
                    printf("parse id error!\n");
                    return;
                }
                g_timer_flag = 0;
                g_add_count = 0;

                code = cJSON_GetObjectItem(json, "data");
                if(code != NULL)
                {
                    printf("base64_decode 1, pn_data.len is %d\n", pn_data.len);
                    de_data_len = base64_decode(code->valuestring, &pn_data.data[pn_data.len]);
                    pn_data.len += de_data_len;
                    g_printing_flag = true;
                    prt_handle.esc_2_prt(ESCPOS_CMD_INIT, 2); 
                    prt_handle.esc_2_prt(prt_list->data, memcmp(prt_list->data + prt_list->len -3, ESCPOS_CMD_CUT1, strlen(ESCPOS_CMD_CUT1)) == 0 ? prt_list->len -3 : prt_list->len);
                    prt_handle.esc_2_prt(pn_data.data, pn_data.len);
                    pn_data.len = 0;
                    if (prt_list->is_receipt && (!prt_list->is_copy))
                    {
                        print_end_string();
                    }
                    destroy_node(prt_list);
                    g_waiting_online_code_flag = 0;
                    printf("prt data 1, clear g_waiting_online_code_flag\n");
                    usleep(10000);
                    //prt_handle.esc_2_prt((unsigned char*)msg->payload.p,(int)msg->payload.len);
                    usleep(10000);
                    //    prt_handle.esc_2_prt(test_feed, 3);
                    prt_handle.printer_cut(96);
                    prt_handle.esc_2_prt(ESCPOS_CMD_INIT, 2); //reset printer before next task to avoid gibberish
                    g_printing_flag = false;
                    //prt_handle.push_printer_process_id(0x01);
                    //prt_handle.printer_cut();
                    //escpos_printer_feed(3);
                    //escpos_printer_cut(1);   
                    cJSON_free(json);
                    cJSON_free(code);
                    // if (pn_data.len > 0)
                    // {
                    //     process_incoming_data(pn_data);
                    // }
                                    
                }                    

              
            }

            }
         
        }
        printf("publish 100\n");
        if(strlen(sub_topic_op) == msg->topic.len)
        {
            if(memcmp(msg->topic.p, sub_topic_op, strlen(sub_topic_op)) == 0)
            {
                printf("op json is: %s\n", msg->payload.p);
                parse_op(msg->payload.p,(int)msg->payload.len);
            }               
        }
        if(strlen(sub_topic_heartbeat) == msg->topic.len)
        {
            printf("rsp topic is:%s\n", sub_topic_heartbeat);
            if(memcmp(msg->topic.p, sub_topic_heartbeat, strlen(sub_topic_op)) == 0)
            {
                printf("heartbeat rsp json is: %s\n", msg->payload.p);
                json = cJSON_Parse(msg->payload.p);
                if(!json)
                {
                    printf("ERROR before: [%s]\n", cJSON_GetErrorPtr());
                }    
                code = cJSON_GetObjectItem(json, "status");
                if(code != NULL)
                {
                    if(code->valueint == 2)
                    {
                        printf("be zerooooooooooooooo!\n");
                        g_downloading_flag = 0;  
                    }

                    if(code->valueint == 0)
                    {
                        printf("heart beat OK!\n!\n"); 
                    }
                                          
                }


            }               
        }
        // cJSON_free(json);
        // cJSON_free(code);

        mqtt_state = 1;
        break;
    }
    case MG_EV_CLOSE:
        printf("Connection closed\n");
        printf("errno = %d\n", errno);
        g_reconnect_flag = 1;
        g_unprint_flag = 1;
        g_offline_flag = 1;
        printf("m_mqtt = %d\n", m_mqtt.active_connections);
    }
}

void mqtt_poll(uint32_t i_time)
{
    //if(pthread_mutex_trylock(&net_lock) == 0)
    if(1)
    {
        mg_mgr_poll(&m_mqtt, i_time);
        //pthread_mutex_unlock(&net_lock);
    }
    else
    {
        printf("wait net_lock!\n");
    }
    

}

void send_heart_beat(void)
{
    int32_t length = 0;
    cJSON *json = NULL;
    struct dirent *filename;  
    DIR * dp;
    struct timeval tv;
    unsigned int file_count = 0;
    char sn[1024 * 2] = { 0 };
    char tmp_buff[32] = {0};

    if(g_offline_flag == 1 || g_unprint_flag == 1)
    {
        return;
    }
   
    load_data("./smart9_status_up.json", sn, &length);    

    
    int cmd = 0;
    json = cJSON_Parse(sn);
    if(!json)
    {
        printf("ERROR before: [%s]\n", cJSON_GetErrorPtr());
    }    

    memset(tmp_buff, 0, sizeof(tmp_buff));
    prt_handle.get_printer_sn(tmp_buff, 32);
    cJSON_ReplaceItemInObject(json, "sn", cJSON_CreateString(tmp_buff));

    
    memset(tmp_buff, 0, sizeof(tmp_buff));
    gettimeofday (&tv, NULL);
    printf("tv_sec; %d\n", tv.tv_sec);
    sprintf(tmp_buff, "%d", tv.tv_sec);

    cJSON_ReplaceItemInObject(json, "datetime", cJSON_CreateString(tmp_buff));    

    memset(tmp_buff, 0, sizeof(tmp_buff));
    sprintf(tmp_buff, "%d", g_downloading_flag);
    cJSON_ReplaceItemInObject(json, "status", cJSON_CreateString(tmp_buff));       

    if(g_net_way == NET_WAY_ETH)
        cJSON_ReplaceItemInObject(json, "network", cJSON_CreateString("2"));
    else if(g_net_way == NET_WAY_WIFI)
        cJSON_ReplaceItemInObject(json, "network", cJSON_CreateString("0"));
    else if(g_net_way == NET_WAY_CELL)
        cJSON_ReplaceItemInObject(json, "network", cJSON_CreateString("1"));

    dp = opendir("/oem/offline_code");
    if(dp != NULL)
    {
        while (filename = readdir(dp))
        {
            file_count++;
        }    
        if(file_count >= 2)       
            file_count -= 2;
        printf("code file count = %d\n", file_count); 
        if(file_count == 0)
        {
            g_offline_prt_flag = 1;
            prt_handle.esc_2_prt("unsynchronized!!!\n", strlen("unsynchronized!!!\n"));
            prt_handle.printer_cut(198);
        }
    }    
    closedir(dp);

    //file_count = 0;//jianfeng
    sprintf(tmp_buff, "%d", file_count);
    cJSON_ReplaceItemInObject(json, "code-remains", cJSON_CreateString(tmp_buff));

    file_count = 0;
    dp = opendir("/oem/offline_data");
    if(dp != NULL)
    {
        while (filename = readdir(dp))
        {
            file_count++;
        }  
        if(file_count >= 2)         
            file_count -= 2;
        printf("data file count = %d\n", file_count); 
    }    

    // if(file_count < 20)
    //     return;

    closedir(dp);
    sprintf(tmp_buff, "%d", file_count);
    cJSON_ReplaceItemInObject(json, "data-stored", cJSON_CreateString(tmp_buff));

    cJSON_ReplaceItemInObject(json, "message", cJSON_CreateString(version));


    memset(sn, 0, sizeof(sn));
    cJSON_PrintPreallocated(json, sn, sizeof(sn), 1);

    printf("heart_beat data is:%s\n", sn);
    printf("m_mqtt = %d\n", m_mqtt.active_connections);
    if(m_mqtt.active_connections != NULL)
        mg_mqtt_publish(m_mqtt.active_connections, pub_topic_heartbeat, 65, MG_MQTT_QOS(0),sn,strlen(sn));
    else
    {
        printf("con == 0000 can`t heartbeat!\n");
    }
    
    if(json != NULL)
        cJSON_free(json);
   
}

uint32_t mqtt_publish_sync(uint32_t topic, char* data, uint32_t *len)
{
    cJSON *json = NULL;
    cJSON *tmp_json = NULL;
    char * content = NULL;
    char * b64_data = NULL;
    int32_t length = 0;
    int32_t tmp = 0;
    char uuid_buf[64] = {0};

    if(g_heart_http_lock == 1)
    {
        prt_handle.esc_2_prt("DEVICE BUSY\n", strlen("DEVICE BUSY\n"));;
        return 0;
    }  

    load_data("./escode/upload.zip", NULL, &length);  
    tmp = length;
    printf("1 need len is:%d\n", length);
    content = (char*)malloc(length * 2);
    b64_data = (char*)malloc(length * 2);
    load_data("./escode/upload.zip", content, &length);
    base64_encode(content,  length, b64_data);

    memset(content, 0, tmp * 2);
    load_data("/oem/upload.json", content, &length);
    json = cJSON_Parse(content);
    if(!json)
    {
        printf("ERROR before: [%s]\n", cJSON_GetErrorPtr());
    }
    memset(g_uuid_buff, 0, sizeof(g_uuid_buff));
    strcpy(g_uuid_buff, GF_GetGUID(uuid_buf));
    cJSON_ReplaceItemInObject(json, "id", cJSON_CreateString(g_uuid_buff));
    strncpy(g_uuid_buff, "00000000", 8);
    if (prt_list)
    {
        strcpy(prt_list->id, g_uuid_buff);
    }
    
    cJSON_ReplaceItemInObject(json, "data", cJSON_CreateString(b64_data));


    memset(content, 0, tmp * 2);
    cJSON_PrintPreallocated(json, content, tmp * 2, 1);   
    printf("upload data is:%s\n", content);

    printf("333\n");


    switch(topic)
    {
        case MQTT_TOPIC_UPLOAD:
            if(g_offline_flag == 1 || g_unprint_flag == 1)
            {
                printf("g_offline_flag == 1 || g_unprint_flag == 1\n");
                g_wait_net_flag = 1;
                return D9_OK;
            }
            g_timer_flag = 1;
            g_timer_count = ONLINE_CODE_TIMEOUT;
            g_overtime_flag = 0;
            printf("m_mqtt.active_connections = %x\n", m_mqtt.active_connections);
            if(m_mqtt.active_connections != 0) {
                mg_mqtt_publish(m_mqtt.active_connections, sub_topic_d9, 65, MG_MQTT_QOS(0),content,strlen(content));
                g_waiting_online_code_flag = 1;
                printf("set g_waiting_online_code_flag\n");
                struct timeval tv;
                gettimeofday (&tv, NULL);
                printf("mg_mqtt_publish time = %ld.%ld\n", tv.tv_sec, tv.tv_usec);
            }
            break;
    }
    
    // while(mqtt_state==0)
    //     mqtt_poll(100);
    mqtt_state = 0;
    free(content);
    free(b64_data);
    return D9_OK;
}

int init_network (void)
{

    printf("start init mqtt!\n");
    //mqtt_init("203.207.198.134.134:61613");
    mqtt_init("121.36.3.243:61613");
    // printf("g_net_status == %d\n", g_net_status);
    // if(g_net_status < 5)
    // {
    //     printf("start 9100 server!\n");
    //     tcp_init("9100"); 
    //     prt_handle.esc_2_prt("9100 start!\n", 13);
    // }
    // if(g_net_status < 4)
    // {
    //     printf("start mqtt connect!\n");
    //     mqtt_init("121.36.3.243:61613");
    //     prt_handle.esc_2_prt("MQTT SERVER CON!\n", 18);
    // }
    // else
    // {
    //     g_offline_flag = 1;
    // }
    
    // prt_handle.printer_cut(128);
    return 0;
}


void *poll_thread(void *arg)
{
    printf("poll_pthread create success!\n");
    while(1)
    {
        if(g_net_status_flag == 10)
        {
            //printf("mqtt poll start---");
            mqtt_poll(100);  
            //printf("---mqtt poll end\n");            
        }   
        if(g_upload_flag == 1)
        {
            g_upload_flag = 0;
            mqtt_publish_sync(MQTT_TOPIC_UPLOAD,"bbb",NULL);
            printf("mqtt_publish end!\n");
        }  
    }
}

void *heart_beat_thread(void *arg)
{
    printf("heart_beat_thread create success!\n");
    while(1)
    {
        if(g_heart_beat_flag == 0x01 && g_heart_http_lock == 0)
        {
            g_heart_beat_flag = 0x00;
            send_heart_beat();
        }   
    }
}

void *prt_task_thread(void *arg)
{
    printf("prt_task_thread create success!\n");
    while(1)
    {
        if(prt_list && (!g_printing_flag) && (!g_waiting_online_code_flag) && (!g_upload_flag) && (!g_heart_http_lock))
        {
            printf("ready to process a prt task with length = %d\n", prt_list->len);
            process_incoming_data(prt_list);
        }
        else 
        {
            usleep(1000 * 1000);
        }   
    }
}

unsigned char parse_op(char *op_json, int len)
{
    cJSON *json = NULL;
    cJSON *opt_code = NULL;
    cJSON *tmp_json = NULL;
    int cmd = 0;
    json = cJSON_Parse(op_json);
    if(!json)
    {
        printf("ERROR before: [%s]\n", cJSON_GetErrorPtr());
    }    

    opt_code = cJSON_GetObjectItem(json, "opt");
    cmd = atoi(opt_code->valuestring);
    switch(cmd)
    {
        case 3:
            g_op_upload_flag = 1;
        break;

        case 4:
            g_op_download_flag = 1;
            tmp_json = cJSON_GetObjectItem(json, "data");
            memset(g_download_url, 0, sizeof(g_download_url));
            strcpy(g_download_url, tmp_json->valuestring);
        break;

        case 5:

        break;

        default:
        break;
    }
    if(json != NULL)
        cJSON_free(json);
    if(opt_code != NULL)
        cJSON_free(opt_code);
    if(tmp_json != NULL)
        cJSON_free(tmp_json);


}

unsigned char parse_http_data(char *http_rsp, int len)
{
    FILE *fp;
    cJSON *json, *tmp_json;
    printf("data %s\n", http_rsp);
    switch(g_http_cmd_flag)
    {
        case 1:
            json = cJSON_Parse(http_rsp);
            if(!json)
            {
                printf("ERROR before: [%s]\n", cJSON_GetErrorPtr());
            } 
            tmp_json = cJSON_GetObjectItem(json, "code");
            if(tmp_json != NULL)
            {
                if(tmp_json->valueint == 0)
                {
                    cJSON_free(json);
                    cJSON_free(tmp_json);
                    return 1;                 
                } 
                else
                {
                    return 0;
                }
                               
            }
            return 0;
          
        break;

        case 2:
            if(http_rsp[0] == '{')
            {
                printf("download error!\n");
                return 1;
            }
            g_heart_http_lock = 1;
            fp = popen("rm /oem/offline_code/offline_code.zip" , "r");
            if(fp != NULL)
            {
                pclose(fp);
            }
            else
            {
                printf("popen faild!!!!!!!!!!\n");
            }            
            dump_data("/oem/offline_code/offline_code.zip", http_rsp, len);  
            printf("start unzip!!!!!!!!!!!!!!!!!!!!!!!!!!\n");  
            fp = popen("unzip -o /oem/offline_code/offline_code.zip -d /oem/offline_code/" , "r");
            if(fp != NULL)
            {
                pclose(fp);
            }
            else
            {
                printf("popen faild!!!!!!!!!!\n");
            } 
            fp = popen("rm /oem/offline_code/offline_code.zip" , "r");
            if(fp != NULL)
            {
                pclose(fp);
            }
            else
            {
                printf("popen faild!!!!!!!!!!\n");
            } 
            fp = popen("sync" , "r");
            if(fp != NULL)
            {
                pclose(fp);
            }
            else
            {
                printf("popen faild!!!!!!!!!!\n");
            }   
            if(g_offline_prt_flag == 1)
            {
                sleep(2);
                g_offline_prt_flag = 0;
                prt_handle.esc_2_prt("synchronized!!!\n", strlen("synchronized!!!\n"));
                prt_handle.printer_cut(198);                
            }  
            g_heart_http_lock = 0;
            return 1;        
        break;

        default:
        break;
    }
    return 0;
}

void event_handler(struct mg_connection *connection, int event_type, void *event_data)
{
	struct http_message *hm = (struct http_message *)event_data;
	int connect_status;
    int i = 0;
    int result = 0;
    int bytesLeft = 0;

    printf("event = %d\n", event_type);
	switch (event_type) {
        case MG_EV_CONNECT:
            connect_status = *(int *)event_data;
            if (connect_status != 0) {
                //printf("Error connecting to server, error code: %d\n", connect_status);
                s_exit_flag = 0;
            }
            break;
        case MG_EV_HTTP_REPLY:
        {
            if(s_exit_flag)
                break;

            result = parse_http_data(hm->body.p, hm->body.len);
            if(result != 0)
            {
                s_exit_flag = result;
            }
            // printf("response len = %d, value: %s\n", hm->body.len ,hm->body.p);
            // for(i = 0; i < hm->body.len; i++)
            // {
            //     if((hm->body.p[i] == '}') && (hm->body.p[i + 1] == '0'))
            //     {
            //         printf("rec end!!!\n");
            //         if (s_exit_flag == 0) 
            //         {
            //             s_exit_flag = 1;
            //         };
            //     }
            // }
            // printf("response len: %d\n", hm->body.len);
            // dump_data("./offline_code.zip", hm->body.p, hm->body.len); 
            // connection->flags |= MG_F_SEND_AND_CLOSE;
            // s_exit_flag = INNER_BREAK_REASON_SUCCEEDED;
        }
            break;
        case MG_EV_HTTP_CHUNK:
        {
            if(s_exit_flag)
                break;
            printf("response len = %d, value: %s\n", hm->body.len ,hm->body.p);
            // for(i = 0; i < hm->body.len; i++)
            // {
            //     if((hm->body.p[i] == '}') && (hm->body.p[i + 1] == '0'))
            //     {
            //         printf("rec end!!!\n");
            //         if (s_exit_flag == 0) 
            //         {
            //             //printf("Server closed connection\n");
            //             s_exit_flag = 1;
            //         };
            //     }
            // }

            
        }
            break;
        case MG_EV_CLOSE:
        {
            if (s_exit_flag == 0) {
                //printf("Server closed connection\n");
                s_exit_flag = 0;
            };
        }
            break;
        default:
            break;
	}
}

void updata_offline_data(void)
{
    FILE *fp;
    cJSON *json = NULL;
    int i;
    unsigned int data_len;
    unsigned char sha1_data[20];
    unsigned char json_data[1024 * 33];   
    struct mg_mgr http_mgr;
    unsigned char *content = NULL;
    char *b64_data = NULL;
    char *tmp_ptr = NULL;
    struct mg_connection *connection;  
    struct timeval tv;
    char tmp_buf[64] = {0};  
    unsigned int updata_offset = 0;
    unsigned int updata_index = 0;

    fp = popen("rm /oem/offline_data.zip" , "r");
    if(fp != NULL)
    {
        pclose(fp);
    }
    else
    {
        printf("popen faild!!!!!!!!!!\n");
    }

    fp = popen("cd /oem/offline_data/ && zip -r /oem/offline_data.zip ./*", "r");
    if(fp != NULL)
    {
        pclose(fp);
    }
    else
    {
        printf("popen faild!!!!!!!!!!\n");
    }

    // fp = popen("zip -r /oem/offline_data.zip ./offline_data/*", "r");
    // if(fp != NULL)
    // {
    //     pclose(fp);
    // }
    // else
    // {
    //     printf("popen faild!!!!!!!!!!\n");
    // }

    // fp = popen("cd /oem/", "r");
    // if(fp != NULL)
    // {
    //     pclose(fp);
    // }
    // else
    // {
    //     printf("popen faild!!!!!!!!!!\n");
    // }

    load_data("/oem/offline_data.zip", NULL, &data_len);  
    printf("2 need len is:%d\n", data_len);
    content = (char*)malloc(data_len + 1);
    b64_data = (char*)malloc(data_len * 2);
    load_data("/oem/offline_data.zip", content, &data_len);
    printf("111 %d\n", data_len);
    base64_encode(content,  data_len, b64_data);
    printf("b64 data len = %d data is:%s\n", strlen(b64_data), b64_data);
    sha1(sha1_data, b64_data, strlen(b64_data));
    printf("sha1 data is: \n");
    print_array(sha1_data, 20);
    

    load_data("/oem/http_file/upload_start_request.json", json_data, &data_len);
    json = cJSON_Parse(json_data);
    if(!json)
    {
        printf("ERROR before: [%s]\n", cJSON_GetErrorPtr());
    }

    gettimeofday (&tv, NULL);
    printf("tv_sec; %d\n", tv.tv_sec);
    sprintf(tmp_buf, "%d", tv.tv_sec);
    cJSON_ReplaceItemInObject(json, "datetime", cJSON_CreateString(tmp_buf));

    memset(tmp_buf, 0, sizeof(tmp_buf));
    prt_handle.get_printer_sn(tmp_buf, 32);
    cJSON_ReplaceItemInObject(json, "sn", cJSON_CreateString(tmp_buf));    

    memset(tmp_buf, 0, sizeof(tmp_buf));
    sprintf(tmp_buf, "%d", strlen(b64_data));
    cJSON_ReplaceItemInObject(json, "datalen", cJSON_CreateString(tmp_buf));



    if(strlen(b64_data) > (32 *1024))
    {
        i = strlen(b64_data) / (32 * 1024);
        i++;
        memset(tmp_buf, 0, sizeof(tmp_buf));
        sprintf(tmp_buf, "%d", i);
        cJSON_ReplaceItemInObject(json, "packetcount", cJSON_CreateString(tmp_buf));
    }
    else
    {
        cJSON_ReplaceItemInObject(json, "packetcount", cJSON_CreateString("1"));
    }
    
    memset(tmp_buf, 0, sizeof(tmp_buf));
    for(i = 0; i < 20; i++)
    {
        sprintf(tmp_buf + (i * 2), "%02x", sha1_data[i]);
    }
    cJSON_ReplaceItemInObject(json, "sha1", cJSON_CreateString(tmp_buf));

    cJSON_PrintPreallocated(json, json_data, sizeof(json_data), 1);
    printf("post data is:%s\n", json_data);
    g_uploading_flag = 1;
    s_exit_flag = 0;
	 mg_mgr_init(&http_mgr, NULL);
     //8-9-9-4  5-0-1-0-1
	 connection = mg_connect_http(&http_mgr, event_handler, "http://printer-pro.d9inggroup.cn/offline_upload", "Content-type: application/json\r\n", json_data);
	 mg_set_protocol_http_websocket(connection);
     g_http_cmd_flag = 1;
	 while (s_exit_flag == 0 && g_upload_overtime_flag == 0)
	 	mg_mgr_poll(&http_mgr, 500);
    if(g_upload_overtime_flag == 1)
    {
        g_upload_overtime_flag = 0;
        free(content);
        free(b64_data);
        mg_mgr_free(&http_mgr);
        return;
    }

    printf("upload start success!\n");

    memset(json_data, 0, sizeof(json_data));
    load_data("/oem/http_file/upload_update_request.json", json_data, &data_len);
    json = cJSON_Parse(json_data);
    if(!json)
    {
        printf("ERROR before: [%s]\n", cJSON_GetErrorPtr());
    }    

    memset(tmp_buf, 0, sizeof(tmp_buf));
    prt_handle.get_printer_sn(tmp_buf, 32);
    cJSON_ReplaceItemInObject(json, "sn", cJSON_CreateString(tmp_buf)); 
    memset(tmp_buf, 0, sizeof(tmp_buf));

    if(strlen(b64_data) > (32 *1024))
    {
        while(1)
        {
            gettimeofday (&tv, NULL);
            printf("tv_sec; %d\n", tv.tv_sec);
            sprintf(tmp_buf, "%d", tv.tv_sec);
            cJSON_ReplaceItemInObject(json, "datetime", cJSON_CreateString(tmp_buf));

            memset(tmp_buf, 0, sizeof(tmp_buf));
            sprintf(tmp_buf, "%d", updata_index);
            cJSON_ReplaceItemInObject(json, "index", cJSON_CreateString(tmp_buf));
            cJSON_ReplaceItemInObject(json, "datalen", cJSON_CreateString("32768"));
            
            sha1(sha1_data, b64_data + updata_offset, 32 * 1024);
            memset(tmp_buf, 0, sizeof(tmp_buf));
            for(i = 0; i < 20; i++)
            {
                sprintf(tmp_buf + (i * 2), "%02x", sha1_data[i]);
            }
            cJSON_ReplaceItemInObject(json, "sha1", cJSON_CreateString(tmp_buf));

            tmp_ptr = (char *)malloc(32 * 1024 + 1);
            memset(tmp_ptr, 0, 32 * 1024 + 1);
            memcpy(tmp_ptr, b64_data + updata_offset, 32 * 1024);
            cJSON_ReplaceItemInObject(json, "data", cJSON_CreateString(tmp_ptr));
            cJSON_PrintPreallocated(json, json_data, sizeof(json_data), 1);
            printf("post data is:%s\n", json_data);
            s_exit_flag = 0;
            mg_mgr_free(&http_mgr);
            mg_mgr_init(&http_mgr, NULL);
            connection = mg_connect_http(&http_mgr, event_handler, "http://printer-pro.d9inggroup.cn/offline_upload", "Content-type: application/json\r\n", json_data);
            mg_set_protocol_http_websocket(connection);
            g_http_cmd_flag = 1;
	        while (s_exit_flag == 0 && g_upload_overtime_flag == 0)
                mg_mgr_poll(&http_mgr, 500); 
            if(g_upload_overtime_flag == 1)
            {
                g_upload_overtime_flag = 0;
                free(content);
                free(b64_data);
                mg_mgr_free(&http_mgr);
                return;
            }
            printf("updata index %d success\n", updata_index);
            updata_index++;
            updata_offset += (32 * 1024);
            if((strlen(b64_data) - updata_offset) <= (32 * 1024))
            {
                memset(tmp_buf, 0, sizeof(tmp_buf));
                gettimeofday (&tv, NULL);
                printf("tv_sec; %d\n", tv.tv_sec);
                sprintf(tmp_buf, "%d", tv.tv_sec);
                cJSON_ReplaceItemInObject(json, "datetime", cJSON_CreateString(tmp_buf));  

                memset(tmp_buf, 0, sizeof(tmp_buf));
                sprintf(tmp_buf, "%d", updata_index);
                cJSON_ReplaceItemInObject(json, "index", cJSON_CreateString(tmp_buf));
                memset(tmp_buf, 0, sizeof(tmp_buf));
                sprintf(tmp_buf, "%d", strlen(b64_data) - updata_offset);                
                cJSON_ReplaceItemInObject(json, "datalen", cJSON_CreateString(tmp_buf));          

                sha1(sha1_data, b64_data + updata_offset, strlen(b64_data) - updata_offset);
                memset(tmp_buf, 0, sizeof(tmp_buf));
                for(i = 0; i < 20; i++)
                {
                    sprintf(tmp_buf + (i * 2), "%02x", sha1_data[i]);
                }
                cJSON_ReplaceItemInObject(json, "sha1", cJSON_CreateString(tmp_buf));  

                memset(tmp_ptr, 0, 32 * 1024 + 1);
                memcpy(tmp_ptr, b64_data + updata_offset, strlen(b64_data) - updata_offset);
                cJSON_ReplaceItemInObject(json, "data", cJSON_CreateString(tmp_ptr)); 
                cJSON_PrintPreallocated(json, json_data, sizeof(json_data), 1);   
                printf("post data is:%s\n", json_data);
                s_exit_flag = 0;
                mg_mgr_free(&http_mgr);
                mg_mgr_init(&http_mgr, NULL);
                connection = mg_connect_http(&http_mgr, event_handler, "http://printer-pro.d9inggroup.cn/offline_upload", "Content-type: application/json\r\n", json_data);
                mg_set_protocol_http_websocket(connection);
                g_http_cmd_flag = 1;
                while (s_exit_flag == 0 && g_upload_overtime_flag == 0)
                    mg_mgr_poll(&http_mgr, 500); 
                if(g_upload_overtime_flag == 1)
                {
                    g_upload_overtime_flag = 0;
                    free(content);
                    free(b64_data);
                    mg_mgr_free(&http_mgr);
                    return;
                }
                printf("last packet %d updata success!\n", updata_index);                   
                break;
            
            }
        }
       
    }
    else
    {
        memset(tmp_buf, 0, sizeof(tmp_buf));
        for(i = 0; i < 20; i++)
        {
            sprintf(tmp_buf + (i * 2), "%02x", sha1_data[i]);
        }
        cJSON_ReplaceItemInObject(json, "sha1", cJSON_CreateString(tmp_buf));
        memset(tmp_buf, 0, sizeof(tmp_buf));
        sprintf(tmp_buf, "%d", strlen(b64_data));
        cJSON_ReplaceItemInObject(json, "datalen", cJSON_CreateString(tmp_buf));
        
        memset(tmp_buf, 0, sizeof(tmp_buf));
        gettimeofday (&tv, NULL);
        printf("tv_sec; %d\n", tv.tv_sec);
        sprintf(tmp_buf, "%d", tv.tv_sec);
        cJSON_ReplaceItemInObject(json, "datetime", cJSON_CreateString(tmp_buf));
        cJSON_ReplaceItemInObject(json, "data", cJSON_CreateString(b64_data));    
        
        cJSON_PrintPreallocated(json, json_data, sizeof(json_data), 1);
        printf("post data is:%s\n", json_data);

        s_exit_flag = 0;
        mg_mgr_free(&http_mgr);
        mg_mgr_init(&http_mgr, NULL);
        connection = mg_connect_http(&http_mgr, event_handler, "http://printer-pro.d9inggroup.cn/offline_upload", "Content-type: application/json\r\n", json_data);
        mg_set_protocol_http_websocket(connection);
        g_http_cmd_flag = 1;
        while (s_exit_flag == 0 && g_upload_overtime_flag == 0)
            mg_mgr_poll(&http_mgr, 500); 
        if(g_upload_overtime_flag == 1)
        {
            g_upload_overtime_flag = 0;
            free(content);
            free(b64_data);
            mg_mgr_free(&http_mgr);
            return;
        }
    }





    printf("upload success!\n");

    memset(json_data, 0, sizeof(json_data));
    load_data("/oem/http_file/upload_final_request.json", json_data, &data_len);
    json = cJSON_Parse(json_data);
    if(!json)
    {
        printf("ERROR before: [%s]\n", cJSON_GetErrorPtr());
    }   

    memset(tmp_buf, 0, sizeof(tmp_buf));
    prt_handle.get_printer_sn(tmp_buf, 32);
    cJSON_ReplaceItemInObject(json, "sn", cJSON_CreateString(tmp_buf)); 

    memset(tmp_buf, 0, sizeof(tmp_buf));
    gettimeofday (&tv, NULL);
    printf("tv_sec; %d\n", tv.tv_sec);
    sprintf(tmp_buf, "%d", tv.tv_sec);
    cJSON_ReplaceItemInObject(json, "datetime", cJSON_CreateString(tmp_buf));
    cJSON_PrintPreallocated(json, json_data, sizeof(json_data), 1);

    printf("post data is:%s\n", json_data);
    s_exit_flag = 0;
    mg_mgr_free(&http_mgr);
    mg_mgr_init(&http_mgr, NULL);
    connection = mg_connect_http(&http_mgr, event_handler, "http://printer-pro.d9inggroup.cn/offline_upload", "Content-type: application/json\r\n", json_data);
    mg_set_protocol_http_websocket(connection);
    g_http_cmd_flag = 1;
    while (s_exit_flag == 0 && g_upload_overtime_flag == 0)
        mg_mgr_poll(&http_mgr, 500); 
    if(g_upload_overtime_flag == 1)
    {
        g_upload_overtime_flag = 0;
        free(content);
        free(b64_data);
        mg_mgr_free(&http_mgr);
        return;
    }

    printf("final success!\n");

    g_uploading_flag = 0;
    mg_mgr_free(&http_mgr);
    free(content);
    free(b64_data);
    free(tmp_ptr);
    if(json != NULL)
        cJSON_free(json);
    
    if(s_exit_flag == 1)
    {
        s_exit_flag = 0;
        fp = popen("rm /oem/offline_data/* -r" , "r");
        if(fp != NULL)
        {
            pclose(fp);
        }
        else
        {
            printf("popen faild!!!!!!!!!!\n");
        }        
    }

}

void download_offline_code(void)
{
    int i;
    struct mg_mgr http_mgr;
    struct mg_connection *connection;  

    g_downloading_flag = 1;


    for(i = 0; i < 3; i++)
    {
        g_download_overtime_flag = 0;
        s_exit_flag = 0;        
        mg_mgr_init(&http_mgr, NULL);
        connection = mg_connect_http(&http_mgr, event_handler, g_download_url, NULL, NULL);
        mg_set_protocol_http_websocket(connection);
        g_http_cmd_flag = 2;
        while (s_exit_flag == 0 && g_download_overtime_flag == 0)
            mg_mgr_poll(&http_mgr, 500); 
        printf("s_exit_flag == %d && g_download_overtime_flag == %d", s_exit_flag, g_download_overtime_flag);
        if(s_exit_flag != 0)
            break;       
    }
    if(g_download_overtime_flag == 1)
    {
        g_downloading_flag = 2;
    }
    else
    {
        g_downloading_flag = 0;
    }
    
    mg_mgr_free(&http_mgr);


    printf("download success!\n");    
}

void *offline_op_thread(void* arg)
{
    while(1)
    {
        if(g_op_upload_flag == 1)
        {
            printf("start upload!\n");
            g_op_upload_flag = 0;
            g_unprint_flag = 1;
            updata_offline_data();
            g_unprint_flag = 0;
        }
        if(g_op_download_flag == 1)
        {
            printf("start download!, url is: %s\n", g_download_url);            
            g_op_download_flag = 0;
            g_unprint_flag = 1;
            download_offline_code();
            g_unprint_flag = 0;
        }

    }


}

char post_buffer[1024*32] = { 0 };

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp) {
    printf("post result: %s\n", buffer);
    memcpy(post_buffer,buffer,strlen(buffer));
    printf("cp\n");
    return strlen(buffer);
}

bool curl_post(char* url,char* content, char* result)
{
    CURL *curl;
    CURLcode res;;
    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, content);    // ָ��post����
        curl_easy_setopt(curl, CURLOPT_URL, url);   // ָ��url
        //curl_easy_setopt(curl, CURLOPT_WRITEDATA,fp);
	    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        res = curl_easy_perform(curl);
        printf("perform\n");
        curl_easy_cleanup(curl);

    }
    memcpy(result,post_buffer,strlen(post_buffer));
    printf("cp2\n");
    return true;
}

size_t curlWriteFunction(void *ptr, size_t size, size_t nmemb, FILE *stream)  
{  
    return fwrite(ptr, size, nmemb, stream);  
}


int download_progress_func(char *progress_data,
                     double t, /* dltotal */
                     double d, /* dlnow */
                     double ultotal,
                     double ulnow)
{
  printf("%s %g / %g (%g %%)\n", progress_data, d, t, d*100.0/t);
  return 0;
}

bool curl_download(char* url, char *filename)
{
    CURL *curl;
    CURLcode res;
    FILE *fp;
    char *progress_data = "* ";
    if ((fp = fopen(filename, "w")) == NULL)  // ���ؽ�����ļ��洢
        return false;

    curl = curl_easy_init();    // ��ʼ��
    if (curl)
    {
       
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteFunction);

        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, download_progress_func);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, progress_data);

        res = curl_easy_perform(curl);   // ִ��
        if (res != 0) {
             printf("res !=0 %d\n", res);
            curl_easy_cleanup(curl);
        }
        fclose(fp);
        return true;
    }
}