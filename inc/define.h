#include "type.h"
#ifndef _D9DEFINE
#define _D9DEFINE

#define D9_OK           0
#define D9_ERROR        13

#define PRT_NET_MAX_BUFF    1024*1024*8 //8MB

#define MQTT_TOPIC_HEARTBEAT        1
#define MQTT_TOPIC_UPLOAD           2

typedef struct _prt_net_data{
    uint8_t data[PRT_NET_MAX_BUFF];
    uint32_t len;
    uint8_t is_handle;
}prt_net_data;
#endif