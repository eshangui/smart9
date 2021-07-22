#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <linux/sockios.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>

#include "NetStatus.h"


typedef int BOOL;
#define TRUE 1
#define FALSE 0

typedef struct _NetStatus {
    char ifName[256];
    int connStat;
    int flagStat;
    int mqttStat;
    int mqttLatency;
} NetStatus;


#ifndef WIN32
#define WSACleanup(...)
#define SOCKET int
#define WSADATA int

void pxat_ns_closesocket(int fd)
{
    close(fd);
}
#endif

static char ns_ipTargetServer[100] = {0};
static char ns_ipFlagServer[100] = {0};
static int ns_flagPort = 0;
static int ns_TargetPort = 0;
static NetStatus ns_netStatus[16] = {0};
static int ns_netCount = 16;
static int ns_timeout = 0;
static int ns_interval = 0;
static pthread_t th[16] = {0};
static pthread_attr_t attr[16] = {0};

int ns_ThreadRunning = 0;

int pxat_ns_diff(struct timeval tic, struct timeval toc)
{
    struct timeval temp;
    long tv_sec, tv_usec;
    if ((toc.tv_usec-tic.tv_usec)<0)
    {
        tv_sec = toc.tv_sec-tic.tv_sec-1;
        tv_usec = 1000000+toc.tv_usec-tic.tv_usec;
    }
    else
    {
        tv_sec = toc.tv_sec-tic.tv_sec;
        tv_usec = toc.tv_usec-tic.tv_usec;
    }
    return tv_sec * 1000 + tv_usec / 1000;
}

int pxat_ns_get_ip(const char* hostname, char *szIp)  
{  
    int iRtn = STATUS_UNKNOWN;
    struct hostent *pHost = gethostbyname(hostname);  
    if(pHost)  
    {  
        if(pHost->h_addrtype == AF_INET)  
        {  
            while ( pHost->h_addr_list[0] )  
            {  
                struct in_addr *pAddr = (struct in_addr*)pHost->h_addr_list[0];
                sprintf(szIp, "%s", inet_ntoa(*pAddr));
                iRtn = STATUS_OK;
                break;
            }  
        }  
    }  

    return iRtn;  
}

int pxat_ns_check_online(char* ip, int port, char* ifName, int* latency)
{
    SOCKET conn_sock;  
    struct sockaddr_in remote_addr;  
    WSADATA wsaData;  

    int error=-1, len;
    len = sizeof(int);
    struct timeval tm, tvStart, tvEnd;
    fd_set set;
    int iRtn = STATUS_UNKNOWN, iRet = 0;

#ifdef WIN32
    WSAStartup(MAKEWORD(1,1),&wsaData);//1.1版本  
#endif  
    conn_sock = socket(AF_INET, SOCK_STREAM, 0);

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(struct ifreq));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s",ifName);
    iRet = ioctl(conn_sock, SIOCGIFINDEX, &ifr);
    iRet = setsockopt(conn_sock, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(struct ifreq));
    if ( iRet < 0) {
           //perror("SO_BINDTODEVICE failed");
           return iRtn;
    }

    remote_addr.sin_family = AF_INET;       
    remote_addr.sin_port = htons(port);
    remote_addr.sin_addr.s_addr = inet_addr(ip);
    unsigned long ul = 1;
    iRet = ioctl(conn_sock, FIONBIO, &ul);

    

    gettimeofday(&tvStart, NULL);
    iRet = connect(conn_sock, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
    if( iRet < 0)
    {
        if (errno == EINPROGRESS)
        {
            tm.tv_sec = ns_timeout / 1000000;
            tm.tv_usec = ns_timeout % 1000000;
            FD_ZERO(&set);
            FD_SET(conn_sock, &set);
            iRet = select(conn_sock+1, NULL, &set, NULL, &tm);
            if( iRet > 0)
            {
                getsockopt(conn_sock, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
                if(error == 0) {
                    iRtn = STATUS_OK;
                }
            }
            else
            {
                iRet = errno;
            }
            
        }        
    }
    else iRtn = STATUS_OK;
    gettimeofday(&tvEnd, NULL);
    if (latency) {
        *latency = pxat_ns_diff(tvStart, tvEnd);
    }
    
    //printf("tvStart sec: %ld, usec:%ld, tvEnd sec: %ld, usec:%ld\n", tvStart.tv_sec, tvStart.tv_usec, tvEnd.tv_sec, tvEnd.tv_usec);
    ul = 0;
    iRet = ioctl(conn_sock, FIONBIO, &ul);
    close( conn_sock );
    return iRtn;
}

int pxat_ns_get_netstat(const char* name)
{
    char    buffer[BUFSIZ];
    FILE    *read_fp;
    int        chars_read;
    int        iRtn;
   
    memset( buffer, 0, BUFSIZ );
    sprintf( buffer, "ifconfig %s | grep RUNNING", name );
    read_fp = popen(buffer, "r");
    if ( read_fp != NULL )
    {
        memset( buffer, 0, BUFSIZ );
        chars_read = fread(buffer, sizeof(char), BUFSIZ-1, read_fp);
        if (chars_read > 0)
        {
            iRtn = NET_STATUS_NETWORK_CONNECTED;
        }
        else
        {
            iRtn = NET_STATUS_NETWORK_NOT_CONNECTED;
        }
        pclose(read_fp);
    }
    else
    {
        iRtn = NET_STATUS_NETWORK_NOT_CONNECTED;
    }

    return iRtn;
}

void* ThreadProc(void *param) {
    int status = 0;
    NetStatus *stat = (NetStatus*)param;
    while (ns_ThreadRunning) {
        //
        stat->connStat = pxat_ns_get_netstat(stat->ifName);
        if (stat->connStat == NET_STATUS_NETWORK_CONNECTED)
        {
            status = pxat_ns_check_online(ns_ipFlagServer, ns_flagPort, stat->ifName, NULL);
            stat->flagStat = status == STATUS_OK ? NET_STATUS_NETWORK_REACHABLE : NET_STATUS_NETWORK_NOT_REACHABLE;
            //printf("ns_ipFlagServer pxat_ns_check_online returns %X, stat->flagStat is %X\n", status, stat->flagStat);
            status = pxat_ns_check_online(ns_ipTargetServer, ns_TargetPort, stat->ifName, &(stat->mqttLatency));
            stat->mqttStat = status == STATUS_OK ? NET_STATUS_SERVER_REACHABLE : NET_STATUS_SERVER_NOT_REACHABLE;
            //printf("ns_ipTargetServer pxat_ns_check_online returns %X, stat->flagStat is %X\n", status, stat->mqttStat);
        }
        else
        {
            stat->flagStat = NET_STATUS_NETWORK_NOT_REACHABLE;
            stat->mqttStat = NET_STATUS_SERVER_NOT_REACHABLE;
        }
        usleep(ns_interval);
    }
    *(int*)param = 0;
    return param;
}

int PXAT_NS_Initialize(const char **ifList, const int ifCount, 
               const char *flagServer, NET_NAME_TYPE flagType, const int flagPort,
               const char *mqttServer, NET_NAME_TYPE mqttType, const int mqttPort,
               const int timeout, const int detectInterval) {
    int iRtn = STATUS_UNKNOWN;
    int i = 0;
    
    if (ifCount <= 0 || ifCount >= 16)
    {
        iRtn = STATUS_ARGUMENT_BAD;
        goto END;
    }

    ns_timeout = timeout * 1000;
    ns_interval = detectInterval * 1000;

    ns_netCount = ifCount;
    memset(ns_netStatus, 0, 16 * sizeof(NetStatus));
    for ( i = 0; i < ifCount; i++)
    {
        strcpy(ns_netStatus[i].ifName, ifList[i]);
    }

    if (flagType == TYPE_DOMAIN_NAME)
    {
        iRtn = pxat_ns_get_ip(flagServer, ns_ipFlagServer);
        if (iRtn != STATUS_OK)
        {
            goto END;
        }
    }
    else
    {
        strcpy(ns_ipFlagServer, flagServer);
    }

    if (mqttType == TYPE_DOMAIN_NAME)
    {
        iRtn = pxat_ns_get_ip(mqttServer, ns_ipTargetServer);
        if (iRtn != STATUS_OK)
        {
            goto END;
        }
    }
    else
    {
        strcpy(ns_ipTargetServer, mqttServer);
    }
    ns_flagPort = flagPort;
    ns_TargetPort = mqttPort;
    ns_ThreadRunning = 1;
    iRtn = STATUS_OK;
    for (i = 0; i < ifCount; i++)
    {
        pthread_attr_init(attr + i);  
        pthread_attr_setdetachstate(attr + i, PTHREAD_CREATE_DETACHED);
        pthread_attr_setscope(attr + i, PTHREAD_SCOPE_SYSTEM );
        pthread_create(th + i, attr + i, ThreadProc, ns_netStatus + i);
        usleep(1000);
    }
    
END:
    return iRtn;
}

int PXAT_NS_Finalize() {
    ns_ThreadRunning = 0;
    return STATUS_OK;
}

int PXAT_NS_GetNetStatus(const char* ifName, int* netStatus, int* latency) {
    int iRtn = STATUS_INVALID_IF_NAME;
    int i = 0;
    for ( i = 0; i < ns_netCount; i++) {
        if (strcmp(ifName, ns_netStatus[i].ifName) == 0) {
            *netStatus = ns_netStatus[i].connStat | ns_netStatus[i].flagStat | ns_netStatus[i].mqttStat;
            if (latency) {
                *latency = ns_netStatus[i].mqttLatency;
            }
            
            iRtn = STATUS_OK;
            break;
        }
    }
    return iRtn;
}

// int main()
// {
//     int i=1, status = 0, count = 10, latency = 0;
//     const char* ifList[] = {"wlan0", "usb0"};
//     printf("!!!!!!!!--------------!!!!!!!!!!!!!\n");
//     while(i)
//     {
//         i = PXAT_NS_Initialize(ifList, 2, "www.baidu.com", TYPE_DOMAIN_NAME, 80, "106.75.115.116", TYPE_IP_ADDRESS, 61613, 5000, 1000);
//         printf("while------Initialize return %X\n", i);
//         usleep(1000 * 1000);
//     }
//     printf("Initialize return %X\n", i);
//     while(1) {
//         i = PXAT_NS_GetNetStatus("wlan0", &status, &latency);
//         printf("PXAT_NS_GetNetStatus return %X, status is %X, latency is %dms\n", i, status, latency);
//         i = PXAT_NS_GetNetStatus("usb0", &status, &latency);
//         printf("usb0PXAT_NS_GetNetStatus return %X, status is %X, latency is %dms\n", i, status, latency);
//         usleep(1000 * 2000);
//     }
    
//     i = PXAT_NS_Finalize();
//     printf("PXAT_NS_Finalize return %X\n", i);
//     return 0;
// }
