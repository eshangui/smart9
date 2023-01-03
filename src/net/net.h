#ifndef _NET_H_
#define _NET_H_

#include "mongoose.h"
//#include "var.h"


#define INNER_BREAK_REASON_SUCCEEDED 0x1

#define INNER_BREAK_REASON_1 0x10 // connect fail
#define INNER_BREAK_REASON_2 0x20 // buffer too small
#define INNER_BREAK_REASON_3 0x30 // server closed
#define INNER_BREAK_REASON_4 0x40 // download fail

extern unsigned char g_upload_flag;

//tcp group provice TCP server for RAW printer
uint32_t tcp_init(const char* port);
void tcp_poll(uint32_t i_time);
void tcp_handler(struct mg_connection *nc, int ev, void *p);

//mqtt provide the channel to server
uint32_t mqtt_init(const char* address);
uint32_t mqtt_free(struct mg_mgr *m);
void mqtt_poll(uint32_t i_time);
void mqtt_handler(struct mg_connection *nc, int ev, void *p);
uint32_t mqtt_publish_sync(uint32_t topic, char* data, uint32_t *len);
int init_network (void);
void *poll_thread(void *arg);
void *heart_beat_thread(void *arg);
void *prt_task_thread(void *arg);
void *offline_op_thread(void *arg);
unsigned char parse_op(char *op_json, int len);
void process_incoming_data(pdata_node prt_data);
int search_cut_ends(unsigned char *prt, int len);

bool curl_download(char* url, char *filename);
bool curl_post(char* url,char* content, char* result);

#endif