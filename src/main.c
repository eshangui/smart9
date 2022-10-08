#include "global.h"
#include "net.h"
#include "prt.h"
#include "uart.h"
#include <pthread.h>
#include "esc2bmp.h"
#include "db_api.h"
#include "common_var.h"
/*
d9main
main service to maintain biz logic of 3308 device
task:
1. check all task status, generate main error code if someone fails.
2. waitng top event like initialize and data exchange and dispatch them to process
3. TBD
*/

void print_init_info(void)
{
     FILE *fp = NULL;
     char mac_buff[64] = {0};
     char prt_mac_buff[64] = {0};
     prt_handle.esc_2_prt(D9MAIN_VERSION, strlen(D9MAIN_VERSION));
     prt_handle.esc_2_prt(g_prt_sn, strlen(g_prt_sn));   


     fp = popen( "ifconfig wlan0 | grep -o -E '([[:xdigit:]]{1,2}:){5}[[:xdigit:]]{1,2}'", "r" );
     if(fp != NULL)
     {
          memset( mac_buff, 0, sizeof(mac_buff) );
          while ( NULL != fgets(mac_buff, sizeof(mac_buff), fp ))
          {
               dbg_printf("wlan mac=%s\n",mac_buff);
               break;
          }  
          memset(prt_mac_buff, 0 , sizeof(prt_mac_buff));
          strcat(prt_mac_buff, "WIFI: ");
          strcat(prt_mac_buff, mac_buff);
          prt_handle.esc_2_prt(prt_mac_buff, strlen(prt_mac_buff));
          pclose(fp); 
     }

     fp = popen( "ifconfig eth0 | grep -o -E '([[:xdigit:]]{1,2}:){5}[[:xdigit:]]{1,2}'", "r" );
     if(fp != NULL)
     {
          memset( mac_buff, 0, sizeof(mac_buff) );
          while ( NULL != fgets(mac_buff, sizeof(mac_buff), fp ))
          {
               dbg_printf("wlan mac=%s\n",mac_buff);
               break;
          }  
          memset(prt_mac_buff, 0 , sizeof(prt_mac_buff));
          strcat(prt_mac_buff, "ETH: ");
          strcat(prt_mac_buff, mac_buff);
          prt_handle.esc_2_prt(prt_mac_buff, strlen(prt_mac_buff));
          pclose(fp); 
     }

     prt_handle.printer_cut(198);         

}

// char g_mqtt_addr[256] = {0};
// char g_mqtt_port[256] = {0};
// char g_mqtt_username[256] = {0};
// char g_mqtt_password[256] = {0};
// char g_upload_addr[256] = {0};

// char g_mqtt_full_addr[256] = {0};
uint32_t load_config()
{
     unsigned char buf[256 * 5] = {0};
     int len = sizeof(buf);
     memset(buf, 0, sizeof(buf));
     bool ret = load_conf("/oem/addr.conf", buf, &len);
     
     if ((!ret) || (len != sizeof(buf)) || (strlen(buf) == 0)
          || (strlen(buf + 256) == 0) || (strlen(buf + 256 * 2) == 0)
          || (strlen(buf + 256 * 3) == 0) || (strlen(buf + 256 * 4) == 0))
     {
          prt_handle.esc_2_prt(CONFIG_LOAD_FAILED_TIP, strlen(CONFIG_LOAD_FAILED_TIP));
          prt_handle.esc_2_prt(D9MAIN_VERSION, strlen(D9MAIN_VERSION));
          prt_handle.esc_2_prt(g_prt_sn, strlen(g_prt_sn));   
          prt_handle.printer_cut(198);   
          return 1;
     }
     memcpy(g_mqtt_addr, buf, 256);
     memcpy(g_mqtt_port, buf + 256, 256);
     memcpy(g_mqtt_username, buf + 256 * 2, 256);
     memcpy(g_mqtt_password, buf + 256 * 3, 256);
     memcpy(g_upload_addr, buf + 256 * 4, 256);
     g_mqtt_addr_type = g_mqtt_port[255] == 0 ? 0 : g_mqtt_port[255] - '0'; // use the last byte of port number to store addr type
     g_mqtt_port[255] = 0;
     g_mqtt_port_num = atoi(g_mqtt_port);
     snprintf(g_mqtt_full_addr, sizeof(g_mqtt_full_addr), "%s:%d", g_mqtt_addr, g_mqtt_port_num);
     dbg_printf("g_mqtt_addr: %s\n", g_mqtt_addr);
     dbg_printf("g_mqtt_port: %s\n", g_mqtt_port);
     dbg_printf("g_mqtt_port_num: %d\n", g_mqtt_port_num);
     dbg_printf("g_mqtt_username: %s\n", g_mqtt_username);
     dbg_printf("g_mqtt_password: %s\n", g_mqtt_password);
     dbg_printf("g_upload_addr: %s\n", g_upload_addr);
     dbg_printf("g_mqtt_full_addr: %s\n", g_mqtt_full_addr);
     return 0;
}


int main(int argc, char **argv)
{
     int i = 0;
     FILE *fp;
    pthread_t p_ble_read;
    pthread_t p_timer;
    pthread_t p_poll;
    pthread_t p_heart_beat;
    pthread_t p_prt_task;
    pthread_t p_offline_op;
    pthread_t p_cellular_op;
    char server_ip[32] = {0};
    char tmp_buff[128] = {0};

    
    prt_init();

    prt_handle.esc_2_prt(ESCPOS_CMD_INIT, strlen(ESCPOS_CMD_INIT));
    if (load_config() != 0)
    {
         mprintf(0,"load_config error\n");
         return 0; 
    }    
    dbg_printf("load config finished\n");
    if(powerup() != 0)
    {
         mprintf(0,"powerup error\n");
         return 0;      
    }
    dbg_printf("power up finished\n");
    if(selfcheck() != 0)
    {
         mprintf(0,"selfcheck error\n");
         return 0;      
    }
    dbg_printf("self check finished\n");
    if(data_sync() != 0)
    {
         mprintf(0,"sync error\n");
         return 0;      
    }
    dbg_printf("data sync finished\n");
    
//chester: add curl http sample function here
//     printf("get URL\n");
//     bool rv = curl_download("http://www.pixelauth.com/UpLoad/Images/202008/10e4252d59da4185996c04b89020c5f6.png","/oem/test.png");
//      if(rv)
//           printf("get done\n");
//      else
//           printf("get fail");
//      char result[1024*32] = { 0 };
//      rv =   ("http://httpbin.org/post","hello world",result);
//       if(rv)
//       {
//             printf("get done\n");
//             printf("main: %s\n",result);
//       }
//      else
//           printf("get fail");

//     while(1)
//     {
         
//     }
//chester: close main body for test
     

    pthread_create(&p_ble_read, NULL, ble_read_thread, NULL);
    pthread_detach(p_ble_read);
    pthread_create(&p_timer, NULL, timer_thread, NULL);
    pthread_detach(p_timer);
    pthread_create(&p_poll, NULL, poll_thread, NULL);
    pthread_detach(p_poll);    
    pthread_create(&p_heart_beat, NULL, heart_beat_thread, NULL);
    pthread_detach(p_heart_beat);
    pthread_create(&p_prt_task, NULL, prt_task_thread, NULL);
    pthread_detach(p_prt_task);
    pthread_create(&p_offline_op, NULL, offline_op_thread, NULL);
    pthread_detach(p_offline_op);
    if (g_sim_flag)
    {
         pthread_create(&p_cellular_op, NULL, cellular_work_thread, NULL);
         pthread_detach(p_cellular_op);
    }
    else
    {
         dbg_printf("sim not detected, cellular thread will not run\n");
    }
    

    while (1)
    {
          if(g_tcp_flag == 3)
          {
               tcp_poll(100);
          }
          if(g_net_change_flag == 1)
          {
               g_net_status_flag = 1;               
               g_unprint_flag = 1;
               g_net_change_flag = 0;
               if(g_init_flag == 0)
               {
                    g_init_flag = 1;
               }
               else
               {
                    sleep(20);                    
               }
               


               if(m_mqtt.active_connections != 0)
                    mqtt_free(&m_mqtt);
               //sleep(10);
               //mqtt_init("203.207.198.134:61613");
               mqtt_init(g_mqtt_full_addr);
               g_reconnect_flag = 0;
               //if(g_net_status_flag == 0)
                    
          }

          if(g_reconnect_flag == 2)
          {
               dbg_printf("start reconnect!\n");
               g_reconnect_flag = 0;
               g_unprint_flag = 1;
               g_offline_flag = 1;
               if(m_mqtt.active_connections != 0)
                    mqtt_free(&m_mqtt);
               //mqtt_init("203.207.198.134:61613");
               mqtt_init(g_mqtt_full_addr);
          }

          if(g_net_status_flag == 1)
          {
               g_net_status_flag = 10;

               if(g_net_way == NET_WAY_ETH)
               {
                    if(g_status_print_flag == 0)
                    {
                         g_status_print_flag = 1;
                         prt_handle.esc_2_prt("---ETH READY---\n", 17);
                         print_init_info();                 
                      
                    }

               }
               if(g_net_way == NET_WAY_WIFI)
               {
                    if(g_status_print_flag == 0)
                    {
                         g_status_print_flag = 1;
                         prt_handle.esc_2_prt("---WIFI READY---\n", 18);
                         print_init_info();                   
                      
                    }

               }
               if(g_net_way == NET_WAY_CELL)
               {
                    if(g_status_print_flag == 0)
                    {
                         g_status_print_flag = 1;
                         prt_handle.esc_2_prt("---4G READY---\n", 16);
                         print_init_info();                  
                        
                    }

               }
          }  
          if(g_net_status_flag == 2)
          {
               dbg_printf("offline!!!\n");
               g_net_status_flag = 10;
               prt_handle.esc_2_prt("---READY---\n", strlen("---READY---"));
               print_init_info();  
          }
          if(g_tcp_flag == 1 || g_tcp_flag == 2)
          {
               dbg_printf("start 9100 server!\n");
               tcp_init("9100");
               prt_handle.esc_2_prt("---NET-PRT_READY---\n", strlen("---NET-PRT_READY---\n") + 1);

               if(g_tcp_flag == 1)
               {
                    fp = popen( "ifconfig eth0 | grep Mask|cut -f 2 -d :", "r" );
               }
               if(g_tcp_flag == 2)
               {
                    fp = popen( "ifconfig wlan0 | grep Mask|cut -f 2 -d :", "r" );
               }
               if(fp != NULL)
               {
                    memset( server_ip, 0, sizeof(server_ip) );
                    while ( NULL != fgets(server_ip, sizeof(server_ip), fp ))
                    {
                         dbg_printf("ip=%s\n",server_ip);
                         break;
                    }              
                    dbg_printf("ip len = %d\n", strlen(server_ip));
                    for(i = 0; i < (strlen(server_ip)); i++)
                    {
                         printf("%02X ", server_ip[i]);
                         if(server_ip[i] == 0x20)
                              break;
                    }
                    printf("\n");
                    server_ip[i] = '\n';
                    prt_handle.esc_2_prt(server_ip, i + 1);
                    g_tcp_flag = 3; 
                    pclose(fp);                   
               }
               else
               {
                    dbg_printf("get 9100 ip faild!\n");
               }

          }


    }
}

