#include "global.h"
#include "powerup.h"
#include "cJSON.h"
//#include "hafs.h"
#include <string.h>

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

    h_file = fopen("./smart9_scheme.json", "rb+");
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
        system("route add -net 203.207.198.0 netmask 255.255.255.0 dev usb0");
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

unsigned char check_net_init(void)
{
    int ret=NET_FAILD, status = 0, count = 10, latency = 0;
    int wlan_ret, celler_ret;
    int wlan_status, celler_status;
    const char* ifList[] = {"wlan0", "usb0"};
    while(ret)
    {
        ret = PXAT_NS_Initialize(ifList, 2, "www.baidu.com", TYPE_DOMAIN_NAME, 80, "203.207.198.134", TYPE_IP_ADDRESS, 61613, 5000, 1000);
        printf("while------Initialize return %X\n", ret);
        usleep(1000 * 1000);
    }
    printf("Initialize return %X\n", ret);
    while(count--) {
        wlan_ret = PXAT_NS_GetNetStatus("wlan0", &wlan_status, &latency);
        printf("PXAT_NS_GetNetStatus return %X, status is %X, latency is %dms\n", wlan_ret, wlan_status, latency);
        celler_ret = PXAT_NS_GetNetStatus("usb0", &celler_status, &latency);
        printf("usb0PXAT_NS_GetNetStatus return %X, status is %X, latency is %dms\n", celler_ret, celler_status, latency);
        if(wlan_ret == 0 && celler_ret == 0)
        {
            if(celler_status == 0x07 && wlan_status == 0x07)
                return NET_ALL_OK;
            else
            {
                if(celler_status == 0x07)
                {
                    if((wlan_status & 0x01) == 0x01)
                        ret = NET_CELL_OK;
                    else
                        ret = ONLY_CELL_OK;
                                        
                }

                if(wlan_status == 0x07)
                    ret = NET_WLAN_OK;                
            }
            

        }
        usleep(1000 * 1000);
    }
    if((wlan_status & 0x01) == 0x01)
        ret = NET_WLAN_CON; 

    //ret = NET_FAILD; 
    return ret;

    
    ret = PXAT_NS_Finalize();
    printf("PXAT_NS_Finalize return %X\n", ret);
    return 0;    
}
