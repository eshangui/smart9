#include "global.h"
#include "powerup.h"
#include "cJSON.h"
//#include "hafs.h"
#include <string.h>
#include "net.h"
#include <termios.h>


int g_usb_fd;
char server_ip[64] = {0};

uint32_t powerup(void)
{
    usleep(100000);
    mprintf(0,"powerup: device init ok \n");
    usleep(100000);
    mprintf(0,"powerup: no ukey connected \n");

    g_net_status = check_net_init();
    var_init();
    return D9_OK;
}

uint32_t var_init(void)
{
    uint32_t ret = 0;
    ret = config_init();

    return D9_OK;
}

uint32_t config_init(void)
{
    int i;
    FILE *h_file = NULL;
    long len = 0;
    char *content = NULL;
    cJSON *json, *json_ip;

    h_file = fopen("/oem/smart9_scheme.json", "rb+");
    fseek(h_file, 0, SEEK_END);
    len = ftell(h_file);
    content = (char*)malloc(len + 1);
    fseek(h_file, 0, SEEK_SET);
    fread(content, 1, len, h_file);
    fclose(h_file);

    printf("addr = %p, str = %s\n", content, content);

    json = cJSON_Parse(content);
    if(!json)
    {
        printf("ERROR before: [%s]\n", cJSON_GetErrorPtr());
    }
    char *str = cJSON_Print(json);

    json_ip = cJSON_GetObjectItem(json, "config");
    json_ip = cJSON_GetObjectItem(json_ip, "network");
    json_ip = cJSON_GetObjectItem(json_ip, "priority");
    //json_ip = cJSON_GetArrayItem(json_ip, 0);

    if(strcmp(json_ip->valuestring, "cellar") == 0)
    {
        // system("route add -net 203.207.198.0 netmask 255.255.255.0 dev usb0");
        // printf("change network to 4G\n");
        // system("ifconfig wlan0 down");
        // system("ifconfig usb0 up");
        // sleep(1);
        // system("echo -e \"AT^NDISDUP=1,1\r\n\" > /dev/ttyUSB0");
        // sleep(1);
        // system("udhcpc -i usb0");
    }
    else
    {
        //tcp_init("9100");
    }
    
    free(content);
    cJSON_free(json);
    cJSON_free(json_ip);
    return D9_OK;
}


#define  PROCBUFSIZ                  1024
#define _PATH_PROC_NET_DEV          "/proc/net/dev"

static char * interface_name_cut(char *buf, char **name)
{
	char *stat;
	/* Skip white space.  Line will include header spaces. */
	while (*buf == ' ')
	buf++;
	*name = buf;
	/* Cut interface name. */
	stat = strrchr (buf, ':');
	*stat++ = '\0';
	return stat;
}

static int check_interface_fromproc(char *interface)
{
  FILE *fp;
  char buf[PROCBUFSIZ];
  struct interface *ifp;
  char *name;

  /* Open /proc/net/dev. */
  fp = fopen (_PATH_PROC_NET_DEV, "r");
  if (fp == NULL)
  {
      printf("open proc file error\n");
      return -1;
   }

  /* Drop header lines. */
  fgets (buf, PROCBUFSIZ, fp);
  fgets (buf, PROCBUFSIZ, fp);

  /* Only allocate interface structure.  Other jobs will be done in if_ioctl.c. */
  while (fgets (buf, PROCBUFSIZ, fp) != NULL)
    {
      interface_name_cut (buf, &name);
      if(strcmp(interface, name) == 0)
          return 1;
    }
  fclose(fp);
  return 0;
}


unsigned char usb_read(int usb_fd, unsigned char *rec_buff)
{
    int count = 0;
    int fd;
    int nread, i;
    unsigned char tmp_buff[1024 * 10] = {0};

    while(1)
    {
        nread = read(usb_fd, tmp_buff, sizeof(tmp_buff));
        printf("first read nread = %d\n", nread);
        if(nread > 0)
        {
            while(1)
            {
                i = read(usb_fd, &tmp_buff[nread], sizeof(tmp_buff));
                if(i > 0)
                {
                    printf("i = %d\n", i);
                    nread += i;
                }
                else
                {
                    count++;
                    usleep(1000 * 1);
                }
                if(count == 1000)
                {
                    printf("rec %d byte : \n", nread);
                    for(i = 0; i < nread; i++)
                    {
                        printf("%c", tmp_buff[i]);
                    }
                    printf("\n");
                    memcpy(rec_buff, tmp_buff, nread);   
                    return nread;                 
                }
                
            }
            
        }            
        usleep(1 * 1000);        
    }


        
    return nread;
}

char searial_parameter[32];

int set_opt(int fd, int nSpeed, int nBits, char nEvent, int nStop)
{
	speed_t speed;
    struct termios newtio, oldtio;
    if (tcgetattr(fd, &oldtio) != 0)
    {
        perror("SetupSerial 1");
        return -1;
    }
    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag |= CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;

    switch (nSpeed)
    {
    case 9600:
        speed = B9600;
        strcat(searial_parameter," 9600");
        break;
    case 19200:
    	speed = B19200;
        strcat(searial_parameter," 19200");
        break;
    case 38400:
        speed = B38400;
        strcat(searial_parameter," 38400");
        break;
    case 57600:
        speed = B57600;
        strcat(searial_parameter," 57600");
        break;
    case 115200:
        speed = B115200;
        strcat(searial_parameter," 115200");
        break;
   	case 230400:
        speed = B230400;
        strcat(searial_parameter," 230400");
        break;
	case 460800:
        speed = B460800;
        strcat(searial_parameter," 460800");
        break;
	case 500000:
        speed = B500000;
        strcat(searial_parameter," 500000");
        break;
	case 576000:
        speed = B576000;
        strcat(searial_parameter," 576000");
        break;
	case 921600:
        speed = B921600;
        strcat(searial_parameter," 921600");
        break;
	case 1000000:
        speed = B1000000;
        strcat(searial_parameter," 1000000");
        break;
	case 1152000:
        speed = B1152000;
        strcat(searial_parameter," 1152000");
        break;
	case 1500000:
        speed = B1500000;
        strcat(searial_parameter," 1500000");
        break;
    default:
        speed = B115200;
        strcat(searial_parameter," 115200");
        break;
    }
    
    switch (nBits)
    {
    case 7:
        newtio.c_cflag |= CS7;
        
        break;
    case 8:
        newtio.c_cflag |= CS8;
        break;
    }

    switch (nEvent)
    {
    case 'O': //奇校�?
        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= PARODD;
        newtio.c_iflag |= (INPCK | ISTRIP);
        strcat(searial_parameter," O");
        break;
    case 'E': //偶校�?
        newtio.c_iflag |= (INPCK | ISTRIP);
        newtio.c_cflag |= PARENB;
        newtio.c_cflag &= ~PARODD;
        strcat(searial_parameter," E");
        break;
    case 'N': //无校�?
        newtio.c_cflag &= ~PARENB;
        strcat(searial_parameter," N");
        break;
    }

    cfsetispeed(&newtio, speed);
    cfsetospeed(&newtio, speed);
    if (nStop == 1)
    {
        newtio.c_cflag &= ~CSTOPB;
        strcat(searial_parameter," 1");
    }
    else if (nStop == 2)
    {
        newtio.c_cflag |= CSTOPB;
        strcat(searial_parameter," 2");
    }
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 0;
#if 0
	switch(flow_ctrl)
    {    
       case 0 ://不使用流控制
              newtio.c_cflag &= ~CRTSCTS;
              break;   
       case 1 ://使用�?件流控制
              newtio.c_cflag |= CRTSCTS;
              break;
       case 2 ://使用�?件流控制
              newtio.c_cflag |= IXON | IXOFF | IXANY;
              break;
    }
#endif
    tcflush(fd, TCIFLUSH);
    if ((tcsetattr(fd, TCSANOW, &newtio)) != 0)
    {
        perror("com set error");
        return -1;
    }

    return 0;
}

unsigned char check_celler(void)
{
    int count = 30;    
    int fd;
    int ret;
    int nread, i;
    unsigned char rec_buff[1024 * 10];
    unsigned char tmp[1024 * 10];


    while(count--)
    {
        printf("check USB0 node...count = %d\n", (30 - count));
        ret = check_interface_fromproc("usb0");
        printf("check_interface_fromproc ret = %d\n", ret);
        if(ret == 1)
            break;
        else
        {
            sleep(1);
        }
    }
    if(count == 0x00)
        return 1;

    fd = open ("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        printf ("\n Open Error \n");
        return 0;
    }

    if(set_opt(fd, 115200, 8, 'N', 1) < 0)
    {
        perror("set_opt error");
        return -1;
    }

    ret = write(fd, " AT^CURC=0\r\n", strlen(" AT^CURC=0\r\n") + 1);
    printf("AT^CURC write ret = %d\n", ret);
    memset(rec_buff, 0x00, sizeof(rec_buff));
    //usleep(1 * 1000);
    ret = usb_read(fd, rec_buff);
    if(ret != 0)
    {
        //printf("CURC_REC len = %d----->:%s\n", ret, rec_buff);
    }
    else
    {
        printf("AT_REC Faild\n");
    }

    // sleep(2);
    // printf("-----------------------------------\n");
    // usb_read(fd, rec_buff);
    // printf("===================================\n");
    // sleep(2);

    ret = write(fd, "AT+CPIN?\r\n", strlen("AT+CPIN?\r\n") + 1);
    printf("CPINusb write ret = %d\n", ret);
    memset(rec_buff, 0x00, sizeof(rec_buff));
    //usleep(10 * 1000);
    ret = usb_read(fd, rec_buff);
    if(ret != 0)
    {
        //printf("CPIN_REC len = %d----->:%s\n", ret, rec_buff);
    }
    else
    {
        printf("AT_REC Faild\n");
    }

    ret = write(fd, " AT+CSQ\r\n", strlen(" AT+CSQ\r\n") + 1);
    printf("CSQusb write ret = %d\n", ret);
    memset(rec_buff, 0x00, sizeof(rec_buff));
    //usleep(100 * 1000);
    ret = usb_read(fd, rec_buff);
    if(ret != 0)
    {
        //printf("CSQ_REC len = %d----->:%s\n", ret, rec_buff);
    }
    else
    {
        printf("AT_REC Faild\n");
    }

    ret = write(fd, " AT^NDISDUP=1,1\r\n", strlen(" AT^NDISDUP=1,1\r\n") + 1);
    printf("usb write ret = %d\n", ret);
    memset(rec_buff, 0x00, sizeof(rec_buff));
    //usleep(10 * 1000);
    ret = usb_read(fd, rec_buff);
    if(ret != 0)
    {
        //printf("NDISDUPAT_REC----->:%s\n", rec_buff);
    }
    else
    {
        printf("AT_REC Faild\n");
    }
    //popen("udhcpc -i usb0", "r");

    //close(fd);
    return 0;    
}

void check_net_thread(void)
{
    unsigned int time_count = 0;
    int ret=NET_FAILD, status = 0, count = 10, latency = 0;
    FILE* fp = NULL;
    char get_way[32] = {0};
    char *set_route_head = "route add -host 203.207.198.134 gw ";
    char set_route[128] = {0};
    const char* ifList[] = {"wlan0", "usb0", "eth0"};

    while(ret)
    {
        ret = PXAT_NS_Initialize(ifList, 3, "203.207.198.134", TYPE_IP_ADDRESS, 61613, "203.207.198.134", TYPE_IP_ADDRESS, 61613, 6000, 5000);
        printf("while------Initialize return %X\n", ret);
        usleep(1000 * 1000);
    }
    printf("Initialize return %X\n", ret);

    // while(1)
    // {
    //     ret = PXAT_NS_GetNetStatus("eth0", &status, &latency);    
    //     printf("eth---PXAT_NS_GetNetStatus return %X, status is %X, latency is %dms\n\n", ret, status, latency);    
    //     ret = PXAT_NS_GetNetStatus("wlan0", &status, &latency);        
    //     printf("wlan0---PXAT_NS_GetNetStatus return %X, status is %X, latency is %dms\n\n", ret, status, latency);    
    //     ret = PXAT_NS_GetNetStatus("usb0", &status, &latency);        
    //     printf("usb0---PXAT_NS_GetNetStatus return %X, status is %X, latency is %dms\n\n\n\n", ret, status, latency);    
    //     usleep(3000 * 1000);
    // }
    
    while(1) 
    {
        printf("---------count = %d -----------\n", time_count);
        if(time_count == 20)
        {
            time_count = 30;
            printf("g_net_status_flag == %d\n", g_net_status_flag);
            if(g_net_status_flag == 0)
                g_net_status_flag = 2;
        }
        ret = PXAT_NS_GetNetStatus("eth0", &status, &latency);
        printf("---eth---PXAT_NS_GetNetStatus return %X, status is %X, latency is %dms--g_net_status_flag = %d g_net_way = %d g_net_change_flag = %d\n", ret, status, latency, g_net_status_flag, g_net_way, g_net_change_flag);
        if(ret == 0)
        {
            if((status & 0x01) == 0x01)
            {
                if(g_tcp_flag == 0x00)
                    g_tcp_flag = 1;
            }
        }
        if(ret == 0 && status == 0x07 && (latency < 3000))
        {
            if(g_net_way != NET_WAY_ETH)
            {
                g_net_way = NET_WAY_ETH;
                fp = popen( "route -n|grep eth0|cut -f 10 -d ' '", "r" );
                memset( get_way, 0, sizeof(get_way) );
                while ( NULL != fgets(get_way, sizeof(get_way), fp ))
                {
                    printf("gateway=%s\n",get_way);
                    break;
                }

                count = strlen(get_way);
                get_way[count - 1] = ' ';
                memset(set_route, 0, sizeof(set_route));
                strcat(set_route, set_route_head);
                strcat(set_route, get_way);
                strcat(set_route, "dev eth0");
                printf("set_route---->:%s\n", set_route);
                fp = popen("route del -host 203.207.198.134", "r" );
                fp = popen(set_route, "r" );
                g_offline_flag = 0;
                //sleep(1);
                //mqtt_free(&m_mqtt);
                sleep(2);
                g_net_change_flag = 1;
                //mqtt_init("203.207.198.134:61613");
            }

        }
        else
        {
            ret = PXAT_NS_GetNetStatus("wlan0", &status, &latency);
            printf("---wlan0---PXAT_NS_GetNetStatus return %X, status is %X, latency is %dms--g_net_status_flag = %d g_net_way = %d g_net_change_flag = %d\n", ret, status, latency, g_net_status_flag, g_net_way, g_net_change_flag);
            if(ret == 0)
            {
                if((status & 0x01) == 0x01)
                {
                    if(g_tcp_flag == 0x00)
                        g_tcp_flag = 2;
                }
            }
            if(ret == 0 && status == 0x07 && (latency < 3000))
            {
                if(g_net_way != NET_WAY_WIFI)
                {
                    //pthread_mutex_lock(&net_lock);
                    g_net_way = NET_WAY_WIFI;
                    fp = popen( "route -n|grep wlan0|cut -f 10 -d ' '", "r" );
                    memset( get_way, 0, sizeof(get_way) );
                    while ( NULL != fgets(get_way, sizeof(get_way), fp ))
                    {
                        printf("gateway=%s\n",get_way);
                        break;
                    }

                    count = strlen(get_way);
                    get_way[count - 1] = ' ';
                    memset(set_route, 0, sizeof(set_route));
                    strcat(set_route, set_route_head);
                    strcat(set_route, get_way);
                    strcat(set_route, "dev wlan0");
                    printf("set_route---->:%s\n", set_route);
                    fp = popen("route del -host 203.207.198.134", "r" );
                    fp = popen(set_route, "r" );
                    g_offline_flag = 0;
                    //sleep(1);
                    //mqtt_free(&m_mqtt);
                    sleep(2);
                    g_net_change_flag = 1;
                    //mqtt_init("203.207.198.134:61613");
                }

            }
            else
            {
                ret = PXAT_NS_GetNetStatus("usb0", &status, &latency);
                printf("usb0PXAT_NS_GetNetStatus return %X, status is %X, latency is %dms\n", ret, status, latency);
                if(ret == 0 && status == 0x07 && (latency < 3000))
                {
                    if(g_net_way != NET_WAY_CELL)
                    {
                        //pthread_mutex_lock(&net_lock);
                        g_net_way = NET_WAY_CELL;
                        fp = popen( "route -n|grep usb0|cut -f 10 -d ' '", "r" );
                        memset( get_way, 0, sizeof(get_way) );
                        while ( NULL != fgets(get_way, sizeof(get_way), fp ))
                        {
                            printf("gateway=%s\n",get_way);
                            break;
                        }

                        count = strlen(get_way);
                        get_way[count - 1] = ' ';
                        memset(set_route, 0, sizeof(set_route));
                        strcat(set_route, set_route_head);
                        strcat(set_route, get_way);
                        strcat(set_route, "dev usb0");
                        printf("set_route---->:%s\n", set_route);
                        fp = popen("route del -host 203.207.198.134", "r" );
                        fp = popen(set_route, "r" );
                        g_offline_flag = 0;
                        //sleep(1);
                        //mqtt_free(&m_mqtt);
                        sleep(2);
                        g_net_change_flag = 1;
                        //mqtt_init("203.207.198.134:61613");
                    }
                }    
                else
                {
                    g_offline_flag = 1;
                    g_net_status_flag = 0;
                    g_net_way = NET_WAY_NULL;
                }
                        
            }            
        }
                


        usleep(1000 * 1000);
        if(time_count < 25)
            time_count++;
    }   

}

unsigned char check_net_init(void)
{

    pthread_t p_check_net;
    int celler_ret;

    celler_ret = check_celler();

    //pthread_mutex_init(&net_lock, NULL);
    g_offline_flag = 1;
    pthread_create(&p_check_net, NULL, check_net_thread, NULL);
    pthread_detach(p_check_net);
}



int findChar(char *sz, char c, int occurences) {
	int count = 0;
	char *p = sz;
	while (*p) {
		if (*p == c) {
			count++;
			if (count == occurences) {
				return (p - sz);
			}
		}
		p++;
	}

	return -1;
}

int SetStaticIP(const char *  ifname, const char * ip, const char * netmask, const char* gateway, const char* dns) {
	char* config = "# interface file auto-generated by buildroot\n\nauto lo\niface lo inet loopback\n\n# The wifi network interface\nauto %s\niface %s inet static\n    address %s\n    netmask %s\n    network %s\n    broadcast %s\n    gateway %s\n    dns-nameservers %s";
	char szData [4096] = { 0 };
	char szNetWork [16] = { 0 };
	char szBroadcast [16] = { 0 };
	int index = -1;
	int iRtn = 0;
	FILE *fp = NULL;
	strcpy (szNetWork, ip);
	strcpy (szBroadcast, ip);
	index = findChar(szNetWork, '.', 3);
	if (index == -1) {
		iRtn = -1;
		goto END;
	}
	strcpy(szNetWork + index + 1, "0");
	strcpy(szBroadcast + index + 1, "255");
	
	sprintf(szData, config, ifname, ifname, ip, netmask, szNetWork, szBroadcast, gateway, dns);

	fp = fopen("/etc/network/interfaces", "wb");
	if (!fp) {
		iRtn = -2;
		goto END;
	}
	
	iRtn = fwrite(szData, strlen(szData), 1, fp);
	if (iRtn != 1) {
		iRtn = -3;
		goto END;
	}

END:
	if (fp) 
		fclose(fp);
	return iRtn;
}

//SetStaticIP("wlan0", "192.168.3.100", "255.255.255.0", "192.168.3.1", "192.168.3.1, 8.8.8.8");

