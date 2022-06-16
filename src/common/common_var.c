#include "common_var.h"
#include "mongoose.h"

char version[] = "SECURE_PRT_V10.31_DELAY_20S\n";

struct mg_mgr m_tcp;
struct mg_mgr m_mqtt;

prt_net_data pn_data;
prt_net_data pn_data_buf;

unsigned char g_net_status;


volatile unsigned char g_init_flag = 0;
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
volatile unsigned char g_waiting_online_code_flag = 0;
volatile unsigned char g_printing_flag = 0;

volatile pdata_node prt_list = 0;



char g_uuid_buff[64] = {0};
unsigned char g_download_url[64] = {0};
char g_prt_sn[64] = {0};

pdata_node malloc_node(unsigned char *buf, int len)
{
  pdata_node node = (pdata_node)malloc(sizeof(data_node));
  node->data = (unsigned char *)malloc(len);
  memcpy(node->data, buf, len);
  node->len = len;
  node->is_receipt = false;
  node->next = NULL;
}

void free_node(pdata_node node)
{
  if(!node)
  {
    return;
  }
  if (node->data)
  {
    free(node->data);
  }
  free(node);
}

pdata_node create_node(unsigned char *buf, int len) {
  pdata_node node = malloc_node(buf, len);
  pdata_node current = prt_list;  
  if (!list_lock)
  {
    list_lock = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(list_lock, NULL);
  }
  pthread_mutex_lock(list_lock);
  if (prt_list)
  {
    while (current->next) 
    {
      current = current->next;
    }
    current->next = node;
  }
  else 
  {
    prt_list = node;
  }
  pthread_mutex_unlock(list_lock);
  return node;
}

void destroy_node(pdata_node node)
{
  pdata_node node1;
  pdata_node node2;
  if (!node || !prt_list)
  {
    return;
  }
  pthread_mutex_lock(list_lock);
  if (node == prt_list)
  {
    prt_list = node->next;

    free_node(node);
  }
  else
  {
    node1 = prt_list;
    node2 = prt_list->next;
    while(node2){
      if(node2 == node) {
        node1->next = node2->next;
        free_node(node2);
        break;
      }
      else{
        node1 = node2;
        node2 = node1->next;
      }
    }
  }
  pthread_mutex_unlock(list_lock);
}


pthread_mutex_t net_lock;

pthread_mutex_t* list_lock = NULL;