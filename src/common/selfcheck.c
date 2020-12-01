#include "global.h"
#include "selfcheck.h"
//#include "hafs.h"
#include "uart.h"

uint32_t selfcheck(void)
{
    usleep(100000);
    mprintf(0,"selfcheck prt ok\n");
    usleep(100000);
    mprintf(0,"selfcheck se ok\n");
    usleep(100000);
    mprintf(0,"selfcheck net ok\n");

    //hafs_check();
    ble_check();


    return D9_OK;
}

uint32_t ble_check(void)
{
    int rv = 0;
    int read_len = 0;
    unsigned short len = 0;
    unsigned char getv_data[] = {"AT+LBDADDR?\r\n"};
    //unsigned char test_data[] = {"AT>\x01\x01\x05\x00\x80\x00123\r"};
    unsigned char test_data[] = {"123456789"};
    unsigned char rec_buff[512] = {0};
    rv =  ble_uart_init();
    if(rv != 0)
    {
        printf("ble uart open error!\n");
    }
    else
    {
        // rv = ble_write(getv_data, sizeof(getv_data));
        // if(rv == -1)
        // {
        //     printf("ble uart write error!\n");
        // }
        // else
        // {
        //     usleep(100000);
        //     rv = ble_at_read(rec_buff,&read_len);
        //     printf("ble read len = %d, data is:%s\n", read_len, rec_buff);
        // }
        // tcflush(g_ble_uart_dev, 2);
        // // rv = ble_write(test_data, sizeof(test_data));
        // // sleep(1);
        // // rv = ble_read(rec_buff,10); 
        // // printf("rec data: %s\n", rec_buff);              
    }
    



}

uint32_t hafs_check(void)
{
    // char sn[20] = {0};
    // unsigned long rv, sn_len = sizeof(sn);
    // PXATFS_FIND_DATA file_info;

    // rv = PXATFS_IsDiskExist(0);
    // if(rv != 0)
    // {
    //     printf("no hafs!\n");
    // }
    // else
    // {
    //     rv = PXATFS_GetDevSN(0, sn, &sn_len);
    //     printf("get device sn:%s\n", sn);

    //     sn_len = 0;
    //     rv = PXATFS_GetFileLength("/config/smart9_scheme.json", &sn_len);
    //     //printf("get file len ret = %d, len = %d\n", rv, sn_len);

    //     rv = PXATFS_GetFileInfo(0, "/config/smart9_scheme.json", &file_info);
    //     //printf("get file info ret = %d\n isDir = %d\n file_size = %d\n file_name = %s\n", rv, file_info.isDir, file_info.fileSize, file_info.fileName);
    //     if(rv == 0)
    //     {
    //         rv = PXATFS_CopyHAFileToHost(0, "/config/smart9_scheme.json", "./smart9_scheme.json");
    //         //printf("copy ret = %d\n", rv);
    //     }
    //     else
    //     {
    //         printf("no json file, ERROR!!!!!!!!!!!!!!\n");
    //         while(1);
    //     }
        
    // }
    



    return D9_OK;
}
