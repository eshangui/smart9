#include <stdio.h>
#include <fcntl.h>   /* File Control Definitions           */
#include <termios.h> /* POSIX Terminal Control Definitions */
#include <unistd.h>  /* UNIX Standard Definitions 	   */
#include <errno.h>   /* ERROR Number Definitions           */
#include "uart.h"
#include "net.h"
#include "common_var.h"
#include "dbg.h"
#include "cJSON.h"



int g_dev = 0;
int g_ble_uart_dev = 0;
unsigned char g_ble_data[1024 * 40];

int ble_uart_init(void)
{
    int fd;
    //fd = open ("/dev/ttyS4", O_RDWR | O_NOCTTY | O_NONBLOCK);
    fd = open ("/dev/ttyS4", O_RDWR | O_NOCTTY );
    if (fd < 0) {
        printf ("\n Open Ble Uart Error \n");
        return 1;
    }
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

    SerialPortSettings.c_cflag &= ~CRTSCTS;       /* No Hardware flow Control                         */
    //SerialPortSettings.c_cflag |= CRTSCTS;       /* Hardware flow Control                         */

    SerialPortSettings.c_cflag |= CREAD | CLOCAL; /* Enable receiver,Ignore Modem Control lines       */

    SerialPortSettings.c_iflag &= ~(IXON | IXOFF | IXANY);         /* Disable XON/XOFF flow control both i/p and o/p */
    SerialPortSettings.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); /* Non Cannonical mode                            */

    SerialPortSettings.c_oflag &= ~OPOST; /*No Output Processing   raw  format  output*/

    SerialPortSettings.c_iflag &= ~ICRNL;

    /* Setting Time outs */
    SerialPortSettings.c_cc[VMIN] = 1;  /* Read at least 10 characters */
    SerialPortSettings.c_cc[VTIME] = 20; /* Wait indefinetly   */

    if ((tcsetattr (fd, TCSANOW, &SerialPortSettings)) != 0) /* Set the attributes to the termios structure*/
    {
        printf ("\n  ERROR ! in Setting attributes");
        return 1;
    }
        
    tcflush (fd, TCIFLUSH); /* Discards old data in the rx buffer            */
    g_ble_uart_dev = fd;
    return 0;


}

int uart_init (int *dev){
    int fd;
    int flag;
    //fd = open ("/dev/ttyS1", O_RDWR | O_NOCTTY | O_NONBLOCK);
    fd = open ("/dev/ttyS1", O_RDWR | O_NOCTTY);
    if (fd < 0) {
        printf ("\n Open Error \n");
        return 0;
    }


    flag = fcntl(fd, F_GETFL, 0);
    printf("flag befor:%08X\n", flag);
    if (fcntl(fd, F_SETFL, 0) < 0)
    {
        printf("fcntl failed!\n");
    }
    flag = fcntl(fd, F_GETFL, 0);
    printf("flag after:%08X\n", flag);
   
    // if (isatty(STDIN_FILENO) == 0)
    // {
    //     printf("standard input is not a terminal device\n");
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
        printf ("\n  ERROR ! in Setting attributes");
    tcflush (fd, TCIFLUSH); /* Discards old data in the rx buffer            */
    *dev = fd;
    return 1;
}

int ble_write(unsigned char *data, int len)
{
    int rv = 0;
    int i;
    printf("write len %d, data is:%s\n", len, data);


    rv = write (g_ble_uart_dev, data, len);
    printf("write ret = %d\n", rv);
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
    printf("rec %d %d %d\n", data[0], data[1], data[2]);
    while(1)
    {
        rv = ble_read(&data[3 + i], 1);
        printf(" i  = %d, for read : %d\n", i, data[3 + i]);
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
            printf ("---------init- fail-g_dev=%d------\n", g_dev);
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
        printf ("---------init- fail-g_dev=%d------\n", g_dev);
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
    unsigned int rec_len = 0;
    unsigned char tmp_data = 0;
    char ret_buff[128] = {0};

    printf("ble read pthread creat success!\n");
    while(1)
    {
        ble_read(&tmp_data, 1);
        printf("-0x%02X ", tmp_data);
        if(tmp_data== 0x1b)
        {
            g_ble_data[i] = tmp_data;
            i++;
            ble_read(&tmp_data, 1);
            if(tmp_data = 0x40)
            {
                g_ble_data[i] = tmp_data;
                i++;
                while(1)
                {
                    ble_read(&g_ble_data[i], 1);
                    i++;
                    if(g_ble_data[i - 1] == 0x69 && g_ble_data[i - 2] == 0x1b)
                    {
                        break;                        
                    }
                }
                printf("data len = %d, data is:\n", i);
                print_array(g_ble_data, i);   


                //prt_handle.esc_2_prt(prt_cmd, sizeof(prt_cmd));
                //prt_handle.esc_2_prt(g_ble_data, i);
                //prt_handle.esc_2_lib(g_ble_data, i);   


                memcpy(pn_data.data, g_ble_data, i - 2);
                pn_data.len = i - 2;

                //uart_write (g_ble_data, i);
                //system("rm ./escode/100000000018330045_100000000018330045_0017.bmp");
                fp = popen("rm ./escode/code.bin", "r");
                if(fp != NULL)
                {
                    while(fgets(ret_buff, sizeof(ret_buff), fp) != NULL)
                    {
                        if('\n' == ret_buff[strlen(ret_buff)-1])
                        {
                            ret_buff[strlen(ret_buff)-1] = '\0';
                        }
                        printf("rm ./escode/code.bin = %s\r\n", ret_buff);
                    }
                    pclose(fp);                   
                }
                dump_data("./escode/code.bin", g_ble_data, i);
                prt_handle.esc_2_lib(g_ble_data, i);
                // system("rm ./escode/upload.zip");
                // system("zip -r ./escode/upload.zip ./escode/*");
                // g_upload_flag = 1;
                i = 0;
                printf("end\n");
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
            if(tmp_data = 0x01)
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
        //usleep(1000);     
    }

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
    printf("str:%s\n", content);
    json = cJSON_Parse(content);
    if(!json)
    {
        printf("ERROR before: [%s]\n", cJSON_GetErrorPtr());
        return;
    }
    char *str = cJSON_Print(json); 

    ssid_val = cJSON_GetObjectItem(json, "SSID");
    pwd_val = cJSON_GetObjectItem(json, "passWord");
    if((strcmp(ssid_val->valuestring, "") != 0) && (strcmp(pwd_val->valuestring, "") != 0) )
    {
        printf("set net ssid&pwd\n");    
        sprintf(shell_str, "/oem/test.sh \"%s\" \"%s\"", ssid_val->valuestring, pwd_val->valuestring);
        printf("shell---->%s\n", shell_str);
        fp = popen(shell_str, "r");  
        if(fp != NULL)
        {
            while(fgets(ret_buff, sizeof(ret_buff), fp) != NULL)
            {
                if('\n' == ret_buff[strlen(ret_buff)-1])
                {
                    ret_buff[strlen(ret_buff)-1] = '\0';
                }
                printf("set net work = %s\r\n", ret_buff);
            }
            pclose(fp);
        }

    }

    // ssid_val = cJSON_GetObjectItem(json, "webConfig");
    // pwd_val = cJSON_GetObjectItem(ssid_val, "webDHCP");
    // printf(" webDHCP : %s\n", pwd_val->valuestring);
    // if(strcmp(pwd_val->valuestring, "0") == 0 )
    // {
    //     ip = cJSON_GetObjectItem(ssid_val, "IP");
    //     mask = cJSON_GetObjectItem(ssid_val, "webMask");
    //     dns = cJSON_GetObjectItem(ssid_val, "webDNS");
    //     gate = cJSON_GetObjectItem(ssid_val, "webGate"); 
    //     printf("static data\n %s\n %s\n %s\n %s\n", ip->valuestring, mask->valuestring, dns->valuestring, gate->valuestring);   
    //     SetStaticIP("wlan0", ip->valuestring, mask->valuestring, gate->valuestring, dns->valuestring);   
    // }



    printf("set net end!-------------------------------------\n");
    fp = popen("sync", "r");
    if(fp != NULL)
    {
        while(fgets(ret_buff, sizeof(ret_buff), fp) != NULL)
        {
            if('\n' == ret_buff[strlen(ret_buff)-1])
            {
                ret_buff[strlen(ret_buff)-1] = '\0';
            }
            printf("sync = %s\r\n", ret_buff);
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

    // ssid_val = cJSON_GetObjectItem(json, "webTypePriority");
    // if(strcmp(ssid_val->valuestring, "4g") == 0)
    // {
    //     printf("change network to 4G\n");
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
    //         printf("ERROR before: [%s]\n", cJSON_GetErrorPtr());
    //     }
    //     //char *str = cJSON_Print(json);

    //     json_ip = cJSON_GetObjectItem(cfc_json, "config");
    //     json_ip = cJSON_GetObjectItem(json_ip, "network");
    //     json_ip = cJSON_GetObjectItem(json_ip, "priority");
    //     cJSON_SetValuestring(json_ip, "cellar");

    //     char *str1 = cJSON_Print(cfc_json);

    //     printf("json --->\n%s", str1);
    //     printf("str1 len = %d\n", strlen(str1));

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
    unsigned char reconnect_count = 0;
    while(1)
    {
        if(g_wait_net_flag == 1)
        {
            //g_overtime_flag = 0;
            g_wait_net_flag = 0;
            get_offline_code();
            prt_handle.esc_2_prt(pn_data.data, pn_data.len);
            prt_handle.esc_2_prt("---------DATA COLLECTED--------\n", 33);
            prt_handle.printer_cut(128);            
        }
               
        if(g_timer_flag == 1)
        {
            g_add_count++;
            if(g_add_count == g_timer_count)
            {
                g_add_count = 0;
                g_overtime_flag = 1;
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
            if(http_download_count == 60)
            {
                http_download_count = 0;
                g_download_overtime_flag = 1;
            }       
        }

        if(g_upload_flag == 1)
        {
            http_download_count++;
            if(http_download_count == 60)
            {
                http_download_count = 0;
                g_upload_overtime_flag = 1;
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


        sleep(1);
        heart_beat_count++;
    }
}

