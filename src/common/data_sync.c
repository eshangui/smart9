#include "global.h"
#include "data_sync.h"

uint32_t data_sync(void)
{
    FILE *fp = NULL;
    char sn_buff[32] = {0};
    char creat_pic_str[128] = "touch /oem/escode/";
    usleep(100000);

    fp = popen("rm ./escode/*" , "r");
    if(fp != NULL)
    {
        pclose(fp);
    }
    else
    {
        dbg_printf("popen faild!!!!!!!!!!\n");
    }    
    prt_handle.get_printer_sn(sn_buff, 32);

    strcpy(&creat_pic_str[strlen(creat_pic_str)], sn_buff);

    strcpy(&creat_pic_str[strlen(creat_pic_str)],".bmp");

    dbg_printf("creat str is:%s\n", creat_pic_str);

    fp = popen(creat_pic_str , "r");
    if(fp != NULL)
    {
        pclose(fp);
    }
    else
    {
        dbg_printf("popen faild!!!!!!!!!!\n");
    }    

    mprintf(0,"sync: no data to sync\n");
    return D9_OK;
}
