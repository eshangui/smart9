#ifndef _VAR_H_
#define _VAR_H_

#include "define.h"
#include <pthread.h>

extern struct mg_mgr m_tcp;
extern struct mg_mgr m_mqtt;

extern prt_net_data pn_data;

extern unsigned char g_net_status;

extern volatile unsigned char g_timer_flag;
extern volatile unsigned char g_timer_count;
extern volatile unsigned char g_add_count;
extern volatile unsigned char g_offline_flag;
extern volatile unsigned char g_wait_net_flag;
extern volatile unsigned char g_overtime_flag;
extern volatile unsigned char g_net_way;
extern volatile unsigned char g_net_change_flag;
extern volatile unsigned char g_tcp_flag;
volatile unsigned char g_net_status_flag;

extern pthread_mutex_t net_lock;

#endif