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

int main(int argc, char **argv)
{
     int i = 0;
     FILE *fp;
    pthread_t p_ble_read;
    pthread_t p_timer;
    pthread_t p_poll;
    pthread_t p_heart_beat;
    pthread_t p_offline_op;
    char server_ip[32] = {0};
    char tmp_buff[128] = {0};

    
    prt_init();    

    if(powerup() != 0)
    {
         mprintf(0,"powerup error %d \n");
         return 0;      
    }
    if(selfcheck() != 0)
    {
         mprintf(0,"selfcheck error %d \n");
         return 0;      
    }
    if(data_sync() != 0)
    {
         mprintf(0,"sync error %d \n");
         return 0;      
    }
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
    pthread_create(&p_offline_op, NULL, offline_op_thread, NULL);
    pthread_detach(p_offline_op); 


    while (1)
    {
          if(g_tcp_flag == 3)
          {
               tcp_poll(100);
          }
          if(g_net_change_flag == 1)
          {
               g_unprint_flag = 1;
               g_net_change_flag = 0;
               if(m_mqtt.active_connections != 0)
                    mqtt_free(&m_mqtt);
               //sleep(10);
               //mqtt_init("121.36.3.243:61613");
               mqtt_init("106.75.115.116:61613");
               //if(g_net_status_flag == 0)
                    g_net_status_flag = 1;
          }

          if(g_reconnect_flag == 2)
          {
               g_reconnect_flag = 0;
               mqtt_init("106.75.115.116:61613");
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
                         prt_handle.esc_2_prt(version, strlen(version));
                         prt_handle.esc_2_prt(g_prt_sn, strlen(g_prt_sn));   
                         prt_handle.printer_cut(198);                   
                      
                    }

               }
               if(g_net_way == NET_WAY_WIFI)
               {
                    if(g_status_print_flag == 0)
                    {
                         g_status_print_flag = 1;
                         prt_handle.esc_2_prt("---WIFI READY---\n", 18);
                         prt_handle.esc_2_prt(version, strlen(version));
                         prt_handle.esc_2_prt(g_prt_sn, strlen(g_prt_sn));   
                         prt_handle.printer_cut(198);                   
                      
                    }

               }
               if(g_net_way == NET_WAY_CELL)
               {
                    if(g_status_print_flag == 0)
                    {
                         g_status_print_flag = 1;
                         prt_handle.esc_2_prt("---4G READY---\n", 16);
                         prt_handle.esc_2_prt(version, strlen(version));
                         prt_handle.esc_2_prt(g_prt_sn, strlen(g_prt_sn)); 
                         prt_handle.printer_cut(198);                   
                        
                    }

               }
          }  
          if(g_net_status_flag == 2)
          {
               printf("offline!!!\n");
               g_net_status_flag = 10;
               prt_handle.esc_2_prt("---OFFLINE---\n", 15);
               prt_handle.esc_2_prt(version, strlen(version));
               prt_handle.esc_2_prt(g_prt_sn, strlen(g_prt_sn));
               prt_handle.printer_cut(198);
          }
          if(g_tcp_flag == 1 || g_tcp_flag == 2)
          {
               printf("start 9100 server!\n");
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
                         printf("ip=%s\n",server_ip);
                         break;
                    }              
                    printf("ip len = %d\n", strlen(server_ip));
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
                    printf("get 9100 ip faild!\n");
               }

          }


    }
}

