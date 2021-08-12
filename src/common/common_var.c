#include "common_var.h"
#include "mongoose.h"

char version[] = "SECURE_PRT_V10.17\n";

struct mg_mgr m_tcp;
struct mg_mgr m_mqtt;

prt_net_data pn_data;

unsigned char g_net_status;

volatile unsigned char g_timer_flag = 0;
volatile unsigned char g_timer_count = 0;
volatile unsigned char g_add_count = 0;
volatile unsigned char g_offline_flag = 0;
volatile unsigned char g_wait_net_flag = 0;
volatile unsigned char g_overtime_flag = 0;
volatile unsigned char g_net_way = 0;
volatile unsigned char g_net_change_flag = 0;
volatile unsigned char g_tcp_flag = 0;
volatile unsigned char g_net_status_flag = 0;
volatile unsigned char g_heart_beat_flag = 0;
volatile unsigned char g_op_upload_flag = 0;
volatile unsigned char g_op_download_flag = 0;
volatile unsigned char g_http_cmd_flag = 0;
volatile unsigned char g_heart_http_lock = 0;
volatile unsigned char g_offline_prt_flag = 0;
volatile unsigned char g_downloading_flag = 0;
volatile unsigned char g_download_overtime_flag = 0;
volatile unsigned char g_uploading_flag = 0;
volatile unsigned char g_upload_overtime_flag = 0;
volatile unsigned char g_status_print_flag = 0;
volatile unsigned char g_unprint_flag = 0;
volatile unsigned char g_reconnect_flag = 0;



unsigned char g_download_url[64] = {0};
char g_prt_sn[64] = {0};



pthread_mutex_t net_lock;