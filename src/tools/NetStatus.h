#ifndef _NETSTATUS_H_
#define _NETSTATUS_H_

typedef enum _NET_NAME_TYPE {
    TYPE_IP_ADDRESS = 0,
    TYPE_DOMAIN_NAME = 1
} NET_NAME_TYPE;

#define NET_STATUS_INIT 0x0
#define NET_STATUS_NETWORK_NOT_CONNECTED 0x80010000
#define NET_STATUS_NETWORK_CONNECTED 0x1
#define NET_STATUS_NETWORK_NOT_REACHABLE 0x80020000
#define NET_STATUS_NETWORK_REACHABLE 0x2
#define NET_STATUS_SERVER_NOT_REACHABLE 0x80040000
#define NET_STATUS_SERVER_REACHABLE 0x4

#define STATUS_OK 0x0
#define STATUS_UNKNOWN 0x1
#define STATUS_FINALIZE_FAILED 0x2
#define STATUS_GET_FAILED 0x3
#define STATUS_ARGUMENT_BAD 0x4
#define STATUS_INVALID_IF_NAME 0x5


/**********************************************************************
* Name: PXAT_NS_Initialize
* Desc: Initialize and start network detection
* Input Params:
*    ifList: name strings of network interfaces
*    ifCount: count of network interfaces in ifList
*    flagServer: common server on internet to detect internet is reachable
*    flagType: type of flagServer, ip address or domain name
*    flagPort: port number of flagServer
*    mqttServer: target mqtt server
*    mqttType: type of mqttServer, ip address or domain name
*    mqttPort: port number of mqttServer
*    timeout: network timeout in milliseconds
*    detectInterval: network status detect interval in milliseconds
* Output Params: None
* Return： 
*    STATUS_OK: Initialize done with no error
*    other value means error occured during PXAT_NS_Initialize
***********************************************************************/
int PXAT_NS_Initialize(const char **ifList, const int ifCount, 
               const char *flagServer, NET_NAME_TYPE flagType, const int flagPort,
               const char *mqttServer, NET_NAME_TYPE mqttType, const int mqttPort,
               const int timeout, const int detectInterval);

/**********************************************************************
* Name: PXAT_NS_Finalize
* Desc: Finalize network detection
* Input Params: None
* Output Params: None
* Return： 
*    STATUS_OK: Finalize done with no error
*    other value means error occured during PXAT_NS_Finalize
***********************************************************************/
int PXAT_NS_Finalize();

/**********************************************************************
* Name: pxat_ns_GetNetStatus
* Desc: get network interface detection
* Input Params: 
*     ifName: network interface name
* Output Params:
*     netStatus: network status of given ifName, combinations of NET_STATUS_*
*     latency: time needed for connection established in milliseconds
* Return： 
*    STATUS_OK: network status got with no error
***********************************************************************/
int PXAT_NS_GetNetStatus(const char* ifName, int* netStatus, int* latency);

#endif //_NETSTATUS_H_