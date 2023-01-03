#include "common_var.h"
#include "mongoose.h"
#include "dbg.h"

struct mg_mgr m_tcp;
struct mg_mgr m_mqtt;

prt_net_data pn_data;
prt_net_data pn_data_buf;
prt_net_data pn_buf;

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
volatile unsigned char g_sim_flag = 0;

volatile pdata_node prt_list = 0;
volatile pbuf_node buf_list = 0;

int g_cellular_fd = -1;



char g_uuid_buff[64] = {0};
unsigned char g_download_url[256] = {0};
char g_prt_sn[64] = {0};

char g_mqtt_addr[256] = {0};
char g_mqtt_port[256] = {0};
char g_mqtt_username[256] = {0};
char g_mqtt_password[256] = {0};
char g_upload_addr[256] = {0};
int g_mqtt_addr_type = 0; //refer to enum NET_NAME_TYPE
int g_mqtt_port_num;
char g_mqtt_full_addr[256] = {0};
unsigned char g_tmp_buff[1024 * 10] = {0};

pdata_node malloc_node(unsigned char *buf, int len)
{
  pdata_node node = (pdata_node)malloc(sizeof(data_node));
  node->data = (unsigned char *)malloc(len);
  memcpy(node->data, buf, len);
  memset(node->id, 0, sizeof(node->id));
  node->len = len;
  node->is_receipt = false;
  node->is_copy = false;
  node->is_processed = false;
  node->next = NULL;
  return node;
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
  dbg_printf("create a node with length = %d\n", len);
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

pbuf_node malloc_buf(char *origin_ip)
{
  pbuf_node node = (pbuf_node)malloc(sizeof(buf_node));
  memset(node, 0, sizeof(buf_node));
  strncpy(node->origin_ip, origin_ip, sizeof(node->origin_ip));
  node->len = 0;
  node->next = NULL;
  return node;
}

void free_buf(pbuf_node node)
{
  if(!node)
  {
    return;
  }
  free(node);
}

pbuf_node create_buf(char *origin_ip) {
  pbuf_node node = malloc_buf(origin_ip);
  pbuf_node current = buf_list;  
  if (!buf_list_lock)
  {
    buf_list_lock = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(buf_list_lock, NULL);
  }
  pthread_mutex_lock(buf_list_lock);
  if (buf_list)
  {
    while (current->next) 
    {
      current = current->next;
    }
    current->next = node;
  }
  else 
  {
    buf_list = node;
  }
  pthread_mutex_unlock(buf_list_lock);
  dbg_printf("create a buf with origin_ip = %s\n", origin_ip);
  return node;
}

void destroy_buf(pbuf_node node) {
  pbuf_node node1;
  pbuf_node node2;
  if (!node || !buf_list)
  {
    return;
  }
  pthread_mutex_lock(buf_list_lock);
  if (node == buf_list)
  {
    buf_list = node->next;

    free_buf(node);
  }
  else
  {
    node1 = buf_list;
    node2 = buf_list->next;
    while(node2){
      if(node2 == node) {
        node1->next = node2->next;
        free_buf(node2);
        break;
      }
      else{
        node1 = node2;
        node2 = node1->next;
      }
    }
  }
  pthread_mutex_unlock(buf_list_lock);
}


pthread_mutex_t net_lock;

pthread_mutex_t* list_lock = NULL;
pthread_mutex_t* buf_list_lock = NULL;

const char *g_key_words[] = {
  "official receipt", 
  "struk resmi",
  "grand total",
  "subtotal",
  "sub total",
  "net total",
  "total amount",
  "change",
  "pb1"
};

const int g_key_words_lengths[] = {
  strlen("official receipt"), 
  strlen("struk resmi"),
  strlen("grand total"),
  strlen("subtotal"),
  strlen("sub total"),
  strlen("net total"),
  strlen("total amount"),
  strlen("change"),
  strlen("pb1")
};

const int g_key_words_count = sizeof(g_key_words_lengths) / sizeof(g_key_words_lengths[0]);

const char *g_copy_words[] = {
  "presettlement bill", 
  "tagihan",
  "this is not a receipt",
  "this is not valid proof of payment",
  "bukan bukti pembayaran",
  "mohon di check kembali",
  "due payment",
  "customer bill",
  "guest check",
  "unpaid",
  "unclosed",
  "end of billing",
  "bill not paid",
  "belum terbayar",
  "copy",
  "salinan",
  "reprint",
  "cetak ulang"
};

const int g_copy_words_lengths[] = {
  strlen("presettlement bill"), 
  strlen("tagihan"),
  strlen("this is not a receipt"),
  strlen("this is not valid proof of payment"),
  strlen("bukan bukti pembayaran"),
  strlen("mohon di check kembali"),
  strlen("due payment"),
  strlen("customer bill"),
  strlen("guest check"),
  strlen("unpaid"),
  strlen("unclosed"),
  strlen("end of billing"),
  strlen("bill not paid"),
  strlen("belum terbayar"),
  strlen("copy"),
  strlen("salinan"),
  strlen("reprint"),
  strlen("cetak ulang")
};

const int g_copy_words_count = sizeof(g_copy_words_lengths) / sizeof(g_copy_words_lengths[0]);

