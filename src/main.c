#include "global.h"
#include "net.h"
#include "prt.h"
#include "uart.h"
#include <pthread.h>
#include "esc2bmp.h"
#include "db_api.h"
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
    char server_ip[32] = {0};
    char version[] = "SECURE_PRT_V10.03\n";

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

     prt_init(); 
      
    init_network();
    pthread_create(&p_ble_read, NULL, ble_read_thread, NULL);
    pthread_detach(p_ble_read);
    pthread_create(&p_timer, NULL, timer_thread, NULL);
    pthread_detach(p_timer);
    
    while (1)
    {
          if(g_net_status_flag == 10)
          {
               tcp_poll(100);
               mqtt_poll(100);              
          }

          if(g_upload_flag == 1)
          {
               g_upload_flag = 0;
               mqtt_publish_sync(MQTT_TOPIC_UPLOAD,"bbb",NULL);
               printf("mqtt_publish end!\n");
          }
          if(g_net_change_flag == 1)
          {
               g_net_change_flag = 0;
               mqtt_init("203.207.198.134:61613");
               //if(g_net_status_flag == 0)
                    g_net_status_flag = 1;
          }
          if(g_net_status_flag == 1)
          {
               g_net_status_flag = 10;
               if(g_net_way == NET_WAY_ETH)
               {
                    prt_handle.esc_2_prt("---ETH READY---\n", 17);
                    prt_handle.esc_2_prt(version, strlen(version));
               }
               if(g_net_way == NET_WAY_WIFI)
               {
                    prt_handle.esc_2_prt("---WIFI READY---\n", 18);
                    prt_handle.esc_2_prt(version, strlen(version));
               }
               if(g_net_way == NET_WAY_CELL)
               {
                    prt_handle.esc_2_prt("---4G READY---\n", 16);
                    prt_handle.esc_2_prt(version, strlen(version));
               }
               print_time(server_ip);
               prt_handle.esc_2_prt(server_ip, 10);
               prt_handle.printer_cut(198);                   
          }  
          if(g_net_status_flag == 2)
          {
               g_net_status_flag = 10;
               prt_handle.esc_2_prt("---OFFLINE---\n", 15);
               prt_handle.esc_2_prt(version, strlen(version));
               prt_handle.printer_cut(198);
          }
          if(g_tcp_flag == 1 || g_tcp_flag == 2)
          {
               printf("start 9100 server!\n");
               tcp_init("9100");
               prt_handle.esc_2_prt("---9100 start---\n", 18);

               if(g_tcp_flag == 1)
               {
                    fp = popen( "ifconfig eth0 | grep Mask|cut -f 2 -d :", "r" );
               }
               if(g_tcp_flag == 2)
               {
                    fp = popen( "ifconfig wlan0 | grep Mask|cut -f 2 -d :", "r" );
               }
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
          }


    }
}

