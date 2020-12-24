#ifndef _POWERUP_H_
#define _POWERUP_H_
#include "NetStatus.h"
#include <fcntl.h>   /* File Control Definitions           */
#include <pthread.h>
#include "prt.h"



#define NET_ALL_OK      0x00
#define NET_CELL_OK     0x01
#define NET_WLAN_OK     0x02
#define ONLY_CELL_OK    0x03
#define NET_WLAN_CON    0x04
#define NET_FAILD       0x05

#define NET_WAY_NULL    0x00
#define NET_WAY_WIFI    0x01
#define NET_WAY_CELL    0x02

extern char server_ip[64];

uint32_t powerup(void);
uint32_t var_init(void);
uint32_t config_init(void);
unsigned char check_net_init(void);
unsigned char check_celler(void);

#endif