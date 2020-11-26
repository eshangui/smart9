#ifndef _UART_H_
#define _UART_H_
#include "prt.h"

extern int g_ble_uart_dev;
extern unsigned char g_ble_data[1024 * 20];

int uart_init(int* dev);
int uart_write(unsigned char* data, int len);
int uart_read(unsigned char* data, int len); 
int uart_close();

int ble_uart_init(void);
int ble_write(unsigned char *data, int len);
int ble_read(unsigned char *data, int len);
int ble_at_read(unsigned char* data, int *len);
void *ble_read_thread(void *arg);


#endif