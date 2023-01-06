#include <stdio.h>
#include <fcntl.h>   /* File Control Definitions           */
#include <termios.h> /* POSIX Terminal Control Definitions */
#include <unistd.h>  /* UNIX Standard Definitions 	   */
#include <errno.h>   /* ERROR Number Definitions           */
#include <sys/ioctl.h>   
#include "common_var.h"
#include "uart.h"
#include "net.h"
#include "dbg.h"
#include "cJSON.h"



int g_dev = 0;
int g_ble_uart_dev = 0;
unsigned char g_ble_data[1024 * 40];

int ble_uart_init(void)
{
    int fd;
    //fd = open ("/dev/ttyS4", O_RDWR | O_NOCTTY | O_NONBLOCK);
    unsigned char buf[100];
    int len = sizeof(buf); 
    fd = open ("/dev/ttyS4", O_RDWR | O_NOCTTY );
    if (fd < 0) {
        dbg_printf ("\n Open Ble Uart Error \n");
        return 1;
    }
    usleep(1000000);

    //tcflush (fd, TCIOFLUSH); /* Discards old data in the io buffer            */

    ioctl(fd, TCFLSH, 2);

    /*RX init*/
    struct termios SerialPortSettings; /* Create the structure                          */
    tcgetattr (fd, &SerialPortSettings); /* Get the current attributes of the Serial port */
    /* Setting the Baud rate */
    cfsetispeed (&SerialPortSettings, B115200);                     
    cfsetospeed (&SerialPortSettings, B115200); 
    /* 8N1 Mode */
    SerialPortSettings.c_cflag &= ~PARENB; /* Disables the Parity Enable bit(PARENB),So No Parity   */
    SerialPortSettings.c_cflag &= ~CSTOPB; /* CSTOPB = 2 Stop bits,here it is cleared so 1 Stop bit */
    SerialPortSettings.c_cflag &= ~CSIZE;  /* Clears the mask for setting the data size             */
    SerialPortSettings.c_cflag |= CS8;     /* Set the data bits = 8                                 */

    //SerialPortSettings.c_cflag &= ~CRTSCTS;       /* No Hardware flow Control                         */
    SerialPortSettings.c_cflag |= CRTSCTS;       /* Hardware flow Control                         */

    SerialPortSettings.c_cflag |= CREAD | CLOCAL; /* Enable receiver,Ignore Modem Control lines       */

    SerialPortSettings.c_iflag &= ~(IXON | IXOFF | IXANY);         /* Disable XON/XOFF flow control both i/p and o/p */
    SerialPortSettings.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); /* Non Cannonical mode                            */

    SerialPortSettings.c_oflag &= ~OPOST; /*No Output Processing   raw  format  output*/

    SerialPortSettings.c_iflag &= ~ICRNL;

    /* Setting Time outs */
    SerialPortSettings.c_cc[VMIN] = 0;  /* Read at least 10 characters */
    SerialPortSettings.c_cc[VTIME] = 0; /* Wait indefinetly   */

    if ((tcsetattr (fd, TCSANOW, &SerialPortSettings)) != 0) /* Set the attributes to the termios structure*/
    {
        dbg_printf ("\n  ERROR ! in Setting attributes");
        return 1;
    }
        
    g_ble_uart_dev = fd;

    memset(buf, 0, len);
    ble_write("AT+BTMODE=1\r", strlen("AT+BTMODE=1\r"));
    usleep(2000000);
    len = ble_read(buf, len);
    dbg_printf ("AT+BTMODE=1 command return len: %d\n", len);
    if (len > 0)
    {
        dbg_printf ("AT+BTMODE=1 command return: %s\n", buf);
        //print_array(buf, len);
    }

    len = sizeof(buf);
    memset(buf, 0, len);
    ble_write("AT+BTMODE?\r", strlen("AT+BTMODE?\r"));
    usleep(2000000);
    len = ble_read(buf, len);
    dbg_printf ("AT+BTMODE? command return len: %d\n", len);
    if (len > 0)
    {
        dbg_printf ("AT+BTMODE? command return: %s\n", buf);
    }
    

    return 0;


}

int uart_init (int *dev){
    int fd;
    int flag;
    //fd = open ("/dev/ttyS1", O_RDWR | O_NOCTTY | O_NONBLOCK);
    fd = open ("/dev/ttyS1", O_RDWR | O_NOCTTY);
    if (fd < 0) {
        dbg_printf ("\n Open Error \n");
        return 0;
    }


    flag = fcntl(fd, F_GETFL, 0);
    dbg_printf("flag befor:%08X\n", flag);
    if (fcntl(fd, F_SETFL, 0) < 0)
    {
        dbg_printf("fcntl failed!\n");
    }
    flag = fcntl(fd, F_GETFL, 0);
    dbg_printf("flag after:%08X\n", flag);
   
    // if (isatty(STDIN_FILENO) == 0)
    // {
    //     dbg_printf("standard input is not a terminal device\n");
    // }
    /*RX init*/
    struct termios SerialPortSettings; /* Create the structure                          */
    bzero(&SerialPortSettings, sizeof(SerialPortSettings));
    tcgetattr (fd, &SerialPortSettings); /* Get the current attributes of the Serial port */
    /* Setting the Baud rate */
    cfsetispeed (&SerialPortSettings, B115200);                     
    /* 8N1 Mode */
    SerialPortSettings.c_cflag &= ~PARENB; /* Disables the Parity Enable bit(PARENB),So No Parity   */
    SerialPortSettings.c_cflag &= ~CSTOPB; /* CSTOPB = 2 Stop bits,here it is cleared so 1 Stop bit */
    SerialPortSettings.c_cflag &= ~CSIZE;  /* Clears the mask for setting the data size             */
    SerialPortSettings.c_cflag |= CS8;     /* Set the data bits = 8                                 */

    //SerialPortSettings.c_cflag &= ~CRTSCTS;       /* No Hardware flow Control                         */
    SerialPortSettings.c_cflag |= CRTSCTS;       /* No Hardware flow Control                         */

    SerialPortSettings.c_cflag |= CREAD | CLOCAL; /* Enable receiver,Ignore Modem Control lines       */

    //SerialPortSettings.c_iflag &= ~(IXON | IXOFF | IXANY);         /* Disable XON/XOFF flow control both i/p and o/p */
    //SerialPortSettings.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); /* Non Cannonical mode                            */

    //SerialPortSettings.c_oflag &= ~OPOST; /*No Output Processing   raw  format  output*/

    //SerialPortSettings.c_iflag &= ~ICRNL;

    /* Setting Time outs */
    SerialPortSettings.c_cc[VMIN] = 0;  /* Read at least 10 characters */
    SerialPortSettings.c_cc[VTIME] = 0; /* Wait indefinetly   */

    if ((tcsetattr (fd, TCSANOW, &SerialPortSettings)) != 0) /* Set the attributes to the termios structure*/
        dbg_printf ("\n  ERROR ! in Setting attributes");
    tcflush (fd, TCIFLUSH); /* Discards old data in the rx buffer            */
    *dev = fd;
    return 1;
}

int ble_write(unsigned char *data, int len)
{
    int rv = 0;
    int i;
    dbg_printf("write len %d, data is:%s\n", len, data);


    rv = write (g_ble_uart_dev, data, len);
    dbg_printf("write ret = %d\n", rv);
    return rv;

}

int uart_write (unsigned char *data, int len)
{
    int rv = 0;
    int i = 0;
    unsigned char leave = 0;
    unsigned char time = 0;
    if (g_dev == 0 || g_dev < 0){
        rv = uart_init (&g_dev);
        if (rv == 0){
            return 0;
        }
    }

    // time = len / 512;
    // leave = len % 512; 

    // for(i = 0; i < time; i++)
    // {
    //     rv = write (g_dev, &data[i * 512], 512);
    //         usleep(200000);
    // }

    // rv = write (g_dev, &data[time * 512], leave);
    rv = write (g_dev, data, len);
    return rv;
}

int ble_at_read(unsigned char* data, int *len)
{
    int rv = 0;
    int i = 0;
    rv = ble_read(data, 3);
    dbg_printf("rec %d %d %d\n", data[0], data[1], data[2]);
    while(1)
    {
        rv = ble_read(&data[3 + i], 1);
        dbg_printf(" i  = %d, for read : %d\n", i, data[3 + i]);
        if(data[3 + i] == '\r')
        {
            rv = ble_read(&data[3 + i + 1], 1);
            if(data[3 + i + 1] == '\n')
                break;
        }
        else
        {
            i++;
        }
        
    }
    *len = i + 5;
}

int ble_read(unsigned char *data, int len)
{
    int rv = 0;
    rv = read (g_ble_uart_dev, data, len);
    if (rv < 0)
        return rv;
    return rv;  
}

int uart_read (unsigned char *data, int len){
    int rv = 0;

    int len_get = 0;
    int counter = 0;
    if (g_dev == 0 || g_dev < 0){
        rv = uart_init (&g_dev);
        if (rv == 0){
            dbg_printf ("---------init- fail-g_dev=%d------\n", g_dev);
            return 0;
        }
    }
    rv = read (g_dev, data, len);
    if (rv < 0)
        return rv;
    return rv;
}

int uart_close (){
    close (g_dev);
    return 0;
}

void uart_test(void)
{
    int rv = 0;
    rv = uart_init (&g_dev);
    if (rv == 0){
        dbg_printf ("---------init- fail-g_dev=%d------\n", g_dev);
    }
    while(1)
    {

        usleep(1000);
    }

}

void *ble_read_thread(void *arg)
{   
    int i = 0;
    int ret = 0;
    int bp, bp1, j;
    FILE *fp;
    unsigned int rec_len = 0, offset = 0;
    unsigned char tmp_data = 0;
    char ret_buff[128] = {0};
    unsigned char ctrl_upload_flag = 0;
    unsigned char READY1[] = {0x0D, 0x0A, 0x49, 0x4D, 0x5F, 0x52, 0x45, 0x41, 0x44, 0x59, 0x0D, 0x0A};
    unsigned char READY2[] = {0x0D, 0x0A, 0x49, 0x4D, 0x5F, 0x43, 0x4F, 0x4E, 0x4E, 0x0D, 0x0A};

    dbg_printf("ble read pthread creat success!\n");
    memset(g_ble_data, 0, sizeof(g_ble_data));
    while(1)
    {
        /*
        ble_read(&tmp_data, 1);
        printf("-0x%02X ", tmp_data);
        if(tmp_data== 0x1b)
        {
            g_ble_data[i] = tmp_data;
            i++;
            ble_read(&tmp_data, 1);
            printf("b-0x%02X ", tmp_data);
            if(tmp_data == 0x40)
            {
                g_ble_data[i] = tmp_data;
                i++;
                while(1)
                {
                    ble_read(&g_ble_data[i], 1);
                    printf("b-0x%02X ", g_ble_data[i]);
                    i++;
                    if(g_ble_data[i - 1] == 0x69 && g_ble_data[i - 2] == 0x1b)
                    {
                        break;                        
                    }
                }
                printf("data len = %d, data is:\n", i);
                print_array(g_ble_data, i); 

                create_node(g_ble_data, i); 

                // printf("memcpy addr->pn_data.data = %d g_ble_data = %d, i - 2 = %d\n", pn_data.data, g_ble_data, i - 2);
                // memcpy(pn_data.data, g_ble_data, i - 2);
                // pn_data.len = i - 2;

                // fp = popen("rm ./escode/code.bin", "r");
                // if(fp != NULL)
                // {
                //     while(fgets(ret_buff, sizeof(ret_buff), fp) != NULL)
                //     {
                //         if('\n' == ret_buff[strlen(ret_buff)-1])
                //         {
                //             ret_buff[strlen(ret_buff)-1] = '\0';
                //         }
                //         printf("rm ./escode/code.bin = %s\r\n", ret_buff);
                //     }
                //     pclose(fp);                   
                // }
                // dump_data("./escode/code.bin", g_ble_data, i);
                // prt_handle.esc_2_lib(g_ble_data, i);
                i = 0;
                printf("ble end 1\n");
            }
            else
            {
                i = 0;
            }
            

        }  

        if(tmp_data== 0x1d)
        {
            g_ble_data[i] = tmp_data;
            i++;
            ble_read(&tmp_data, 1);
            printf("d-0x%02X ", tmp_data);
            if(tmp_data == 0x21)
            {
                g_ble_data[i] = tmp_data;
                i++;
                while(1)
                {
                    ble_read(&g_ble_data[i], 1);
                    printf("d-0x%02X ", g_ble_data[i]);
                    i++;
                    // if(strncmp(&g_ble_data[i - strlen("Scan Kode Sid9    ")], "Scan Kode Sid9", strlen("Scan Kode Sid9")) == 0)
                    // {
                    //     printf("need printf4!\n");
                    //     ctrl_upload_flag = 1;
                    // }
                    
                    if(g_ble_data[i - 1] == 0x01 && g_ble_data[i - 2] == 0x56 && g_ble_data[i - 3] == 0x1d)
                    {
                        {
                            printf("only prt rec end!\n");
                            create_node(g_ble_data, i);
                            break;
                        }
                        
                    }
                    // if(strncmp(&g_ble_data[i - strlen("Scan Kode Sid9    ")], "Scan Kode Sid9", strlen("Scan Kode Sid9")) == 0)
                    // {
                    //     printf("need printf4!\n");
                    //     ctrl_upload_flag = 1;
                    // }
                    if(g_ble_data[i - 4] == 0x70 && g_ble_data[i - 5] == 0x1b)
                    {
                        //memcpy(&g_ble_data[i - 8], &g_ble_data[i - 5], 5);
                        create_node(g_ble_data, i);
                        printf("\nrec END!!!!\n");
                        break;                        
                    }

                }

                printf("data is : \n"); 
                for(j = 0; j < i; j++)
                {
                    printf("%c", g_ble_data[j]);
                }
                i = 0;
                printf("\nble end 2\n");
                // if(ctrl_upload_flag == 1)
                // {
                //     ctrl_upload_flag = 0;
                //     memcpy(pn_data.data, g_ble_data, i - 3);
                //     pn_data.len = i - 3;     

                //     fp = popen("rm ./escode/code.bin", "r");
                //     if(fp != NULL)
                //     {
                //         while(fgets(ret_buff, sizeof(ret_buff), fp) != NULL)
                //         {
                //             if('\n' == ret_buff[strlen(ret_buff)-1])
                //             {
                //                 ret_buff[strlen(ret_buff)-1] = '\0';
                //             }
                //             printf("rm ./escode/code.bin = %s\r\n", ret_buff);
                //         }
                //         pclose(fp);                   
                //     }
                //     dump_data("./escode/code.bin", g_ble_data, i);
                //     prt_handle.esc_2_lib(g_ble_data, i);
                //     // system("rm ./escode/upload.zip");
                //     // system("zip -r ./escode/upload.zip ./escode/*");
                //     // g_upload_flag = 1;
                //     i = 0;
                //     printf("end\n");             
                // }
                // else
                // {
                //     prt_handle.esc_2_prt(g_ble_data, i - 3);
                //     prt_handle.printer_cut(96);
                //     i = 0;
                //     printf("only prt end 6\n");
                // }
                


                //prt_handle.esc_2_prt(prt_cmd, sizeof(prt_cmd));
                //prt_handle.esc_2_prt(g_ble_data, i);
                //prt_handle.esc_2_lib(g_ble_data, i);   


                // memcpy(pn_data.data, g_ble_data, i - 2);
                // pn_data.len = i - 2;

                // //uart_write (g_ble_data, i);
                // //system("rm ./escode/100000000018330045_100000000018330045_0017.bmp");
                // fp = popen("rm ./escode/code.bin", "r");
                // if(fp != NULL)
                // {
                //     while(fgets(ret_buff, sizeof(ret_buff), fp) != NULL)
                //     {
                //         if('\n' == ret_buff[strlen(ret_buff)-1])
                //         {
                //             ret_buff[strlen(ret_buff)-1] = '\0';
                //         }
                //         printf("rm ./escode/code.bin = %s\r\n", ret_buff);
                //     }
                //     pclose(fp);                   
                // }
                // dump_data("./escode/code.bin", g_ble_data, i);
                // prt_handle.esc_2_lib(g_ble_data, i);
                // // system("rm ./escode/upload.zip");
                // // system("zip -r ./escode/upload.zip ./escode/*");
                // // g_upload_flag = 1;
                // i = 0;
                // printf("end\n");
            }
            else
            {
                i = 0;
            }
            

        }  
 
        if(tmp_data== 0xEF)
        {
            g_ble_data[i] = tmp_data;
            i++;
            ble_read(&tmp_data, 1);
            if(tmp_data == 0x01)
            {
                g_ble_data[i] = tmp_data;
                i++;
                for(j = 0; j < 7; j++)
                {
                   ble_read(&g_ble_data[i], 1);
                   i += 1; 
                }
                rec_len = g_ble_data[6] * 256 + g_ble_data[7];
                printf("rec_len = %d\n", rec_len);
                for(j = 0; j < rec_len + 2; j++)
                {
                   ble_read(&g_ble_data[i], 1);
                   i += 1; 
                }
                print_array(g_ble_data, rec_len + 10);      
                ret = parse_ble_data(&g_ble_data[9], rec_len - 1);
                i = 0;
                printf("end\n");
            }
            else
            {
                i = 0;
            }
            

        }   
        memset(g_ble_data, 0, sizeof(g_ble_data));
        //usleep(1000);    
        */ 
        i = ble_read(g_ble_data + offset, sizeof(g_ble_data) - offset);
        if (i <= 0)
        {
            //if (offset == 0)
            {
                continue;
            }
        }
        else
        {
            dbg_printf("ble data received, len is %d \n", i);
            print_array(g_ble_data + offset, i);  

            //SKIP "IMREADY"
            if ((i == sizeof(READY1)) && (memcmp(g_ble_data + offset, READY1, sizeof(READY1)) == 0))
            {
                dbg_printf("ble module ready1 msg received, len is %d, current offset is %d, discard it\n", i, offset);
                continue;
            }
            else if ((i == sizeof(READY2)) && (memcmp(g_ble_data + offset, READY2, sizeof(READY2)) == 0))
            {
                dbg_printf("ble module ready2 msg received, len is %d, current offset is %d, discard it\n", i, offset);
                continue;
            }
            offset += i;
        }

        // command structure:
        // 81 XX XX D0 DATA CRC1 CRC2  XX XX is length for the whole data
        // data structure
        //             EF 01 FF FF 00 00 XX XX 50 YY YY YY ... CRC1 CRC2, XX XX is length for 50 YY YY YY ...
        if (g_ble_data[0] == 0x81)
        {
            dbg_printf("may be ble command , offset is %d\n", offset);
            if (offset < 4)
            {
                continue;
            }

            dbg_printf("(g_ble_data[1] << 8 + g_ble_data[2]) is %d\n", ((g_ble_data[1] << 8) + g_ble_data[2]));
            
            if (offset < (((g_ble_data[1] << 8) + g_ble_data[2]))) 
            {
                continue;
            }

            if (g_ble_data[3] != 0xD0)
            {
                dbg_printf("g_ble_data[3] != 0xD0, not a ble command\n");
                offset = 0;
                continue;
            }

            if (g_ble_data[4] == 0xEF && g_ble_data[5] == 0x01) // 
            {
                dbg_printf("g_ble_data[4] == 0xEF && g_ble_data[5] == 0x01\n");
                if (g_ble_data[12] == 0x50)
                {
                    dbg_printf("g_ble_data[12] == 0x50\n");
                    //TODO:: check crc
                    rec_len = g_ble_data[10] * 256 + g_ble_data[11];
                    dbg_printf("rec_len = %d, ble command data is:\n", rec_len);
                    for(j = 0; j < rec_len - 1; j++) 
                    {
                        printf("%c", g_ble_data[13 + j]);
                    }
                    printf("\n");
                    //print_array(g_ble_data, rec_len + 10);      
                    ret = parse_ble_data(&g_ble_data[13], rec_len - 1);
                    dbg_printf("ble cmd end\n");
                }
                i = 0;
                offset = 0;
                memset(g_ble_data, 0, sizeof(g_ble_data));
                continue;
            }
            else
            {
                dbg_printf("not a EF 01 ble command \n");
                offset = 0;
                continue;
            }   
        }        

        // if ((memcmp(ESCPOS_CMD_CUT_0, g_ble_data + offset - 2, strlen(ESCPOS_CMD_CUT_0)) == 0) 
        //     || memcmp(ESCPOS_CMD_CUT_1, g_ble_data + offset - 2, strlen(ESCPOS_CMD_CUT_1)) == 0)
        // {
        //     create_node(g_ble_data, offset);
        //     i = 0;
        //     offset = 0;
        //     memset(g_ble_data, 0, sizeof(g_ble_data));
        //     continue;
        // }
        // else
        // {
            
        // }

        // if((memcmp(g_ble_data + offset - sizeof(ESCPOS_CMD_CUT_0), ESCPOS_CMD_CUT_0, sizeof(ESCPOS_CMD_CUT_0)) == 0)
        //    || (memcmp(g_ble_data + offset - sizeof(ESCPOS_CMD_CUT_1), ESCPOS_CMD_CUT_1, sizeof(ESCPOS_CMD_CUT_1)) == 0) 
        //    || (memcmp(g_ble_data + offset - sizeof(ESCPOS_CMD_CUT0), ESCPOS_CMD_CUT0, sizeof(ESCPOS_CMD_CUT0)) == 0)
        //    || (memcmp(g_ble_data + offset - sizeof(ESCPOS_CMD_CUT1), ESCPOS_CMD_CUT1, sizeof(ESCPOS_CMD_CUT1)) == 0)
        //    || (memcmp(g_ble_data + offset - sizeof(ESCPOS_CMD_CUT2), ESCPOS_CMD_CUT2, sizeof(ESCPOS_CMD_CUT2)) == 0))
        if (search_cut_ends(g_ble_data, offset) >= 0)
        {
            create_node(g_ble_data, offset);
            i = 0;
            offset = 0;
            memset(g_ble_data, 0, sizeof(g_ble_data));
            continue;   
        }
        else
        {
            //if(prt_data.data[len - 4] == 0x70 && prt_data.data[len - 5] == 0x1b)
            dbg_printf("debug 1112\n");
            if ((offset) < 8) {
                continue;
            }
            
            if(memcmp(g_ble_data + offset- 5, ESCPOS_CMD_CASHBOX, strlen(ESCPOS_CMD_CASHBOX)) == 0)
            {
                create_node(g_ble_data, offset);
                i = 0;
                offset = 0;
                memset(g_ble_data, 0, sizeof(g_ble_data));
                continue;     
            }
        }
    }
    dbg_printf("ble thread exits\n");

}

unsigned char parse_ble_data(unsigned char *data, unsigned int len)
{

    FILE *h_file = NULL;
    FILE *fp = NULL;
    long file_len = 0;
    cJSON *cfc_json = NULL;
    cJSON *json_ip = NULL;
    char *content = NULL;
    char ret_buff[128] = {0};
    char shell_str[128] = {0};
    
    cJSON *json = NULL;
    cJSON *ssid_val = NULL;
    cJSON *pwd_val = NULL;

    content = (char*)malloc(len + 1);
    memset(content, 0, len + 1);
    memcpy(content, data, len);
    dbg_printf("str:%s\n", content);
    json = cJSON_Parse(content);
    if(!json)
    {
        dbg_printf("ERROR before: [%s]\n", cJSON_GetErrorPtr());
        return 0xFF;
    }
    char *str = cJSON_Print(json); 

    ssid_val = cJSON_GetObjectItem(json, "SSID");
    pwd_val = cJSON_GetObjectItem(json, "passWord");
    if((strcmp(ssid_val->valuestring, "") != 0) && (strcmp(pwd_val->valuestring, "") != 0) )
    {
        dbg_printf("set net ssid&pwd\n");    
        sprintf(shell_str, "/oem/test.sh \"%s\" \"%s\"", ssid_val->valuestring, pwd_val->valuestring);
        dbg_printf("shell---->%s\n", shell_str);
        fp = popen(shell_str, "r");  
        if(fp != NULL)
        {
            while(fgets(ret_buff, sizeof(ret_buff), fp) != NULL)
            {
                if('\n' == ret_buff[strlen(ret_buff)-1])
                {
                    ret_buff[strlen(ret_buff)-1] = '\0';
                }
                dbg_printf("set net work = %s\r\n", ret_buff);
            }
            pclose(fp);
        }

    }

    // ssid_val = cJSON_GetObjectItem(json, "webConfig");
    // pwd_val = cJSON_GetObjectItem(ssid_val, "webDHCP");
    // dbg_printf(" webDHCP : %s\n", pwd_val->valuestring);
    // if(strcmp(pwd_val->valuestring, "0") == 0 )
    // {
    //     ip = cJSON_GetObjectItem(ssid_val, "IP");
    //     mask = cJSON_GetObjectItem(ssid_val, "webMask");
    //     dns = cJSON_GetObjectItem(ssid_val, "webDNS");
    //     gate = cJSON_GetObjectItem(ssid_val, "webGate"); 
    //     dbg_printf("static data\n %s\n %s\n %s\n %s\n", ip->valuestring, mask->valuestring, dns->valuestring, gate->valuestring);   
    //     SetStaticIP("wlan0", ip->valuestring, mask->valuestring, gate->valuestring, dns->valuestring);   
    // }



    dbg_printf("set net end!-------------------------------------\n");
    fp = popen("sync", "r");
    if(fp != NULL)
    {
        while(fgets(ret_buff, sizeof(ret_buff), fp) != NULL)
        {
            if('\n' == ret_buff[strlen(ret_buff)-1])
            {
                ret_buff[strlen(ret_buff)-1] = '\0';
            }
            dbg_printf("sync = %s\r\n", ret_buff);
        }
        pclose(fp);
    }
    prt_handle.esc_2_prt("Please Reboot!\n", strlen("Please Reboot!\n"));
    prt_handle.printer_cut(196);

    free(content);
    if(json != NULL)
        cJSON_free(json);
    if(ssid_val != NULL)
     cJSON_free(ssid_val);
    if(pwd_val != NULL)
        cJSON_free(pwd_val);
    return 0;

    // ssid_val = cJSON_GetObjectItem(json, "webTypePriority");
    // if(strcmp(ssid_val->valuestring, "4g") == 0)
    // {
    //     dbg_printf("change network to 4G\n");
    //     system("ifconfig wlan0 down");
    //     system("ifconfig usb0 up");
    //     sleep(1);
    //     system("echo -e \"AT^NDISDUP=1,1\r\n\" > /dev/ttyUSB0");
    //     sleep(1);
    //     system("udhcpc -i usb0"); 

    //     h_file = fopen("./smart9_scheme.json", "rb");
    //     fseek(h_file, 0, SEEK_END);
    //     file_len = ftell(h_file);
    //     char *tmp_con = (char*)malloc(file_len + 1);
    //     fseek(h_file, 0, SEEK_SET);
    //     fread(tmp_con, 1, file_len, h_file);
    //     fclose(h_file);
    //     system("rm ./smart9_scheme.json");
    //     cfc_json = cJSON_Parse(tmp_con);
    //     if(!cfc_json)
    //     {
    //         dbg_printf("ERROR before: [%s]\n", cJSON_GetErrorPtr());
    //     }
    //     //char *str = cJSON_Print(json);

    //     json_ip = cJSON_GetObjectItem(cfc_json, "config");
    //     json_ip = cJSON_GetObjectItem(json_ip, "network");
    //     json_ip = cJSON_GetObjectItem(json_ip, "priority");
    //     cJSON_SetValuestring(json_ip, "cellar");

    //     char *str1 = cJSON_Print(cfc_json);

    //     dbg_printf("json --->\n%s", str1);
    //     dbg_printf("str1 len = %d\n", strlen(str1));

    //     h_file = fopen("./smart9_scheme.json", "wb");
    //     fwrite(str1, 1, strlen(str1), h_file);
    //     fclose(h_file); 


    //     free(tmp_con);          
    // }
    // cJSON_free(cfc_json);
    // cJSON_free(json_ip);  
    // cJSON_free(json);  
    // cJSON_free(ssid_val);  
    // cJSON_free(pwd_val);  

}


void *timer_thread(void *arg)
{
    unsigned char heart_beat_count = 0;
    unsigned char http_download_count = 0;
    unsigned char http_upload_count = 0;
    unsigned char reconnect_count = 0;
    int prt_len = 0;
    unsigned char prt_status_count = 0;
    while(1)
    {
        if(g_wait_net_flag == 1)
        {
            //g_overtime_flag = 0;
            g_wait_net_flag = 0; 
            g_waiting_online_code_flag = 0;
            g_printing_flag = true;
            if (prt_list)
            {
                if (prt_list->is_receipt && (!prt_list->is_copy))
                {
                    get_offline_code();
                    dbg_printf("==================start offline prt=================%d\n", prt_list->len);
                    g_printing_flag = true;
                    prt_handle.esc_2_prt(ESCPOS_CMD_INIT, 2);
                    usleep(1000 * 10);
                    memset(pn_buf.data, 0, sizeof(pn_buf.data));
                    pn_buf.len = 0;
                    memcpy(pn_buf.data + pn_buf.len, ESCPOS_CMD_INIT, 2);
                    pn_buf.len += 2;
                    //prt_len = memcmp(prt_list->data + prt_list->len -3, ESCPOS_CMD_CUT1, strlen(ESCPOS_CMD_CUT1)) == 0 ? prt_list->len -3 : prt_list->len;
                    int cut_len = search_cut_ends(prt_list->data, prt_list->len);
                    prt_len = (cut_len < 0) ? prt_list->len : (prt_list->len - cut_len);
                    dbg_printf("======pn_buf.data=0x%08X pn_buf.len=%d prt_list->data=0x%08X prt_len=%d\n", pn_buf.data, pn_buf.len, prt_list->data, prt_len);
                    memcpy(pn_buf.data + pn_buf.len, prt_list->data, prt_len);
                    pn_buf.len += prt_len;
                    memcpy(pn_buf.data + pn_buf.len, pn_data.data, pn_data.len);
                    pn_buf.len += pn_data.len;
                    prt_len += print_end_string(pn_buf.data + pn_buf.len);
                    pn_buf.len += prt_len;
                    // memcpy(pn_buf.data + pn_buf.len, ESCPOS_CMD_INIT, 2);
                    // pn_buf.len += 2;
                    memcpy(pn_buf.data + pn_buf.len, "---------CHECKED OUT--------\n", strlen("---------CHECKED OUT--------\n"));
                    pn_buf.len += strlen("---------CHECKED OUT--------\n");
                    // memcpy(pn_buf.data + pn_buf.len, "\n\n\n\n", 4);
                    // pn_buf.len += 4;
                    memcpy(pn_buf.data + pn_buf.len, ESCPOS_CMD_INIT, 2);
                    pn_buf.len += 2;
                    prt_handle.esc_2_prt(pn_buf.data, pn_buf.len);
                    dbg_printf("prt data 1, clear g_waiting_online_code_flag\n");
                    prt_handle.printer_cut(196);
                    // prt_handle.esc_2_prt(ESCPOS_CMD_INIT, 2); 
                    // prt_handle.esc_2_prt(prt_list->data, memcmp(prt_list->data + prt_list->len -3, ESCPOS_CMD_CUT1, strlen(ESCPOS_CMD_CUT1)) == 0 ? prt_list->len -3 : prt_list->len);
                    // prt_handle.esc_2_prt(pn_data.data, pn_data.len);
                    // print_end_string();
                }
                destroy_node(prt_list);
            }
            
            dbg_printf("offline prt, clear g_waiting_online_code_flag\n");
            pn_data.len = 0;
            // prt_handle.esc_2_prt("---------CHECKED OUT--------\n", 33);
            // prt_handle.printer_cut(128);      
            // prt_handle.esc_2_prt(ESCPOS_CMD_INIT, 2); //reset printer before next task to avoid gibberish
            g_printing_flag = false;
            dbg_printf("prt data 5\n");
            // if (g_waiting_online_code_flag)
            // {
            //     memcpy(pn_data.data, pn_data_buf.data, pn_data_buf.len);
            //     pn_data.len = pn_data_buf.len;
            //     pn_data_buf.len = 0;
            //     g_waiting_online_code_flag = 0;
            //     process_incoming_data(&pn_data, pn_data.len);
            // }
                  
        }
               
        if(g_timer_flag == 1)
        {
            g_add_count++;
            if(g_add_count == g_timer_count)
            {
                g_add_count = 0;
                g_overtime_flag = 1;
                dbg_printf("g_overtime_flag = 1\n"); 
                g_timer_flag = 0;
                g_timer_count = 0;
                g_wait_net_flag = 1;
            }
        }

        if(heart_beat_count == 60)
        {
            heart_beat_count = 0;
            g_heart_beat_flag = 1;

        }

        if(g_downloading_flag == 1)
        {
            http_download_count++;
            if(http_download_count == 600)
            {
                http_download_count = 0;
                g_download_overtime_flag = 1;
            }       
        }
        else
        {
            if (http_download_count)
            {
                http_download_count = 0;
            }   
        }

        if(g_uploading_flag == 1)
        {
            http_upload_count++;
            if(http_upload_count == 300)
            {
                http_upload_count = 0;
                g_upload_overtime_flag = 1;
            }       
        }
        else
        {
            if (http_upload_count)
            {
                http_upload_count = 0;
            }
        }

        if(g_reconnect_flag == 1)
        {
            reconnect_count++;
            if(reconnect_count == 10)
            {
                reconnect_count = 0;
                g_reconnect_flag = 2;
            }

        }

        {
            if (prt_status_count == 10)
            {
                prt_status_count = 0;
                dbg_printf("prt_list=0x%X, g_op_download_flag=%d, g_op_upload_flag=%d, g_printing_flag=%d, g_waiting_online_code_flag=%d, g_upload_flag=%d, g_heart_http_lock=%d\n", prt_list, g_op_download_flag, g_op_upload_flag, g_printing_flag, g_waiting_online_code_flag, g_upload_flag, g_heart_http_lock);
                if (prt_list)
                {
                    dbg_printf("prt_list->is_processed=%d\n", prt_list->is_processed);
                }
            }
            prt_status_count++;
            if (prt_list)
            {
                if(g_op_upload_flag) g_op_upload_flag = 0;
                if(g_op_download_flag) g_op_download_flag = 0;
            }
            
        }

        sleep(1);
        heart_beat_count++;
    }
}

