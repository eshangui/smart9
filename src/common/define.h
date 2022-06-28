#ifndef _DEFINE_H
#define _DEFINE_H

#include <stdint.h>
#define D9_OK           0
#define D9_ERROR        13

#define PRT_NET_MAX_BUFF    1024*1024*8 //8MB

#define MQTT_TOPIC_HEARTBEAT        1
#define MQTT_TOPIC_UPLOAD           2

#define RECEIPT_END_STRING "\n\nScan Kode Sid9 dan\nMenangkan Hadiahnya!\n\n\n\n\n"

//#define D9MAIN_VERSION "SECURE_PRT_V11.06_DELAY_20S\n"
//#define D9MAIN_VERSION "SECURE_PRT_V11.06_TEST_ADDR\n"
#define D9MAIN_VERSION "SECURE_PRT_V11.06\n"

//#define ONLINE_CODE_TIMEOUT         20   // seconds for waiting online code timeout
#define ONLINE_CODE_TIMEOUT         6   // seconds for waiting online code timeout

typedef struct _prt_net_data{
    uint8_t data[PRT_NET_MAX_BUFF];
    uint32_t len;
    uint8_t is_handle;
}prt_net_data;



#endif