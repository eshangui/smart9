#include <stdio.h>
#include "global.h"
#include "prt.h"
#include "net.h"
#include "uart.h"
#include <errno.h>


unsigned char g_upload_flag = 0;
static const char *s_user_name = "admin";
static const char *s_password = "password";
static const char *pub_topic_heartbeat = "/smart9/up/heartbeat/";
static const char *sub_topic_heartbeat = "/smart9/down/heartbeat/1122334455667788";
static const char *pub_topic_upload = "/smart9/up/upload/1122334455667788";
static const char *sub_topic_upload = "/smart9/down/upload/1122334455667788";
static const char *sub_topic_d9 = "UPLOAD";
static const char *sub_topic_prt = "100000000018330045";
static struct mg_mqtt_topic_expression s_topic_expr = {NULL, 0};

uint32_t tcp_init(const char *port)
{
    pn_data.len = 0;
    pn_data.is_handle = 0;
    mg_mgr_init(&m_tcp, NULL);
    mg_bind(&m_tcp, port, tcp_handler);
    printf("Starting tcp mgr on port %s\n", port);
    return D9_OK;
}

void tcp_handler(struct mg_connection *nc, int ev, void *p)
{
    struct mbuf *io = &nc->recv_mbuf;
    static int tcp_rec_len = 0;
    (void)p;

    switch (ev)
    {
    case MG_EV_RECV:
        //mg_send(nc, io->buf, io->len); // Echo message back
        printf("get through socket len = %d\n",(int)io->len);
        print_array(io->buf,io->len);
        memcpy(&pn_data.data[tcp_rec_len], io->buf, io->len);
        tcp_rec_len += io->len;
        mbuf_remove(io, io->len); // Discard message from recv buffer
        if(pn_data.data[tcp_rec_len - 1] == 0x69 && pn_data.data[tcp_rec_len - 2] == 0x1b)
        {
            pn_data.len = tcp_rec_len - 2;
            tcp_rec_len = 0;
            system("rm ./escode/code.bin");
            dump_data("./escode/code.bin", pn_data.data, pn_data.len);
            system("rm ./escode/upload.zip");
            system("zip -r ./escode/upload.zip ./escode/*");
            
            g_upload_flag = 1;            
        }

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
    struct mg_mqtt_message *msg = (struct mg_mqtt_message *)p;
    (void)nc;

    if (ev != MG_EV_POLL) printf("mqtt GOT event %d\n", ev);

    switch (ev) {
    case MG_EV_CONNECT: {
        struct mg_send_mqtt_handshake_opts opts;
        memset(&opts, 0, sizeof(opts));
        opts.user_name = s_user_name;
        opts.password = s_password;

        mg_set_protocol_mqtt(nc);
        mg_send_mqtt_handshake_opt(nc, "dummy", opts);
        break;
    }
    case MG_EV_MQTT_CONNACK:
        if (msg->connack_ret_code != MG_EV_MQTT_CONNACK_ACCEPTED) {
            printf("Got mqtt connection error: %d\n", msg->connack_ret_code);
            exit(1);
        }
        s_topic_expr.topic = sub_topic_d9;
        printf("Subscribing to '%s'\n", sub_topic_d9);
        mg_mqtt_subscribe(nc, &s_topic_expr, 1, 42);
        s_topic_expr.topic = sub_topic_prt;
        printf("Subscribing to '%s'\n", sub_topic_prt);
        mg_mqtt_subscribe(nc, &s_topic_expr, 1, 42);
        pthread_mutex_unlock(&net_lock);
        break;
    case MG_EV_MQTT_PUBACK:
        printf("Message publishing acknowledged (msg_id: %d)\n", msg->message_id);
        break;
    case MG_EV_MQTT_SUBACK:
        printf("Subscription acknowledged\n");
        break;
    case MG_EV_MQTT_PUBLISH: {
        printf("Got incoming message %.*s: %d\n", (int)msg->topic.len,
            msg->topic.p, (int)msg->payload.len);
        print_array((unsigned char*)msg->payload.p,(int)msg->payload.len);
        if(memcmp(msg->topic.p, sub_topic_prt, msg->topic.len) == 0)
        {
           //dump_data("./prt.bin", msg->payload.p,(int)msg->payload.len);
           //prt_print(pn_data.data, pn_data.len);
           //escpos_printer_feed(3);
           if(g_overtime_flag == 0)
           {
                g_timer_flag = 0;
                g_add_count = 0;
                memcpy(&pn_data.data[pn_data.len], (unsigned char*)msg->payload.p, (int)msg->payload.len);
                pn_data.len += (int)msg->payload.len;
                prt_handle.esc_2_prt(pn_data.data, pn_data.len);
                usleep(10000);
                //prt_handle.esc_2_prt((unsigned char*)msg->payload.p,(int)msg->payload.len);
                usleep(10000);
                //    prt_handle.esc_2_prt(test_feed, 3);
                    prt_handle.printer_cut(96);
                    prt_handle.push_process_id(0x01);
                //prt_handle.printer_cut();
                //escpos_printer_feed(3);
                //escpos_printer_cut(1);               
           }

        }
        mqtt_state = 1;
        break;
    }
    case MG_EV_CLOSE:
        printf("Connection closed\n");
        printf("errno = %d\n", errno);
        //exit(1);
    }
}

void mqtt_poll(uint32_t i_time)
{
    if(pthread_mutex_trylock(&net_lock) == 0)
    {
        mg_mgr_poll(&m_mqtt, i_time);
        pthread_mutex_unlock(&net_lock);
    }
    else
    {
        printf("wait net_lock!\n");
    }
    

}

uint32_t mqtt_publish_sync(uint32_t topic, char* data, uint32_t *len)
{
    int32_t length = 0;
    char sn[1024 * 2] = { 0 };
   
    load_data("./escode/upload.zip", sn, &length);
    printf("length = %d\n", length);
    printf("g_offline_flag = %d\n", g_offline_flag);
    switch(topic)
    {
        case MQTT_TOPIC_HEARTBEAT:
            mg_mqtt_publish(m_mqtt.active_connections, pub_topic_heartbeat, 65, MG_MQTT_QOS(0),sn,strlen(sn));
            break;
        case MQTT_TOPIC_UPLOAD:
            if(g_offline_flag == 1)
            {
                g_wait_net_flag = 1;
                return D9_OK;
            }
            g_timer_flag = 1;
            g_timer_count = 6;
            g_overtime_flag = 0;
            mg_mqtt_publish(m_mqtt.active_connections, sub_topic_d9, 65, MG_MQTT_QOS(0),sn,length);
            break;
    }
    
    while(mqtt_state==0)
        mqtt_poll(100);
    mqtt_state = 0;
    return D9_OK;
}

int init_network (void)
{

    printf("start init mqtt!\n");
    mqtt_init("203.207.198.134:61613");
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
    //     mqtt_init("203.207.198.134:61613");
    //     prt_handle.esc_2_prt("MQTT SERVER CON!\n", 18);
    // }
    // else
    // {
    //     g_offline_flag = 1;
    // }
    
    // prt_handle.printer_cut(128);
    return 0;
}