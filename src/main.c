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
    pthread_t p_ble_read;
    pthread_t p_timer;

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
        tcp_poll(100);
        mqtt_poll(100);
        if(g_upload_flag == 1)
        {
          g_upload_flag = 0;
          mqtt_publish_sync(MQTT_TOPIC_UPLOAD,"bbb",NULL);
          printf("mqtt_publish end!\n");
        }

    }
}

