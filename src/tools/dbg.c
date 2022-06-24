#include <stdio.h>
#include <stdarg.h>
#include "type.h"
#include "common_var.h"
//#include "bmp.h"
#include "dbg.h"
#if _WIN32
#include <stdint.h>
#else
#include <unistd.h>
#endif
#include <sys/timeb.h>
#include <sys/time.h>

#define M_PRT
#define M_BMP
#define PRT_TIME

#ifdef PRT_TIME
long long app_start_time = 0;

long long getSystemTime()
{
        struct timeb t;
        ftime(&t);
        return 1000 * t.time + t.millitm;
}
#endif

void mprintf(int32_t level, const char *cmd, ...)
{
#ifdef M_PRT
        va_list args;
#ifdef PRT_TIME
        long long this_print_time;
        if (app_start_time == 0)
        {
                app_start_time = getSystemTime();
                this_print_time = app_start_time;
        }
        else
                this_print_time = getSystemTime();
#endif

#ifdef PRT_TIME
        printf("time: %lld ms -- ", this_print_time-app_start_time);
#endif
PRT:

        va_start(args, cmd);
        vprintf(cmd, args);
        va_end(args);
#endif
}

void ByteToHexStr(const unsigned char *source, char *dest, int sourceLen)
{
        short i;
        unsigned char highByte, lowByte;

        for (i = 0; i < sourceLen; i++)
        {
                highByte = source[i] >> 4;
                lowByte = source[i] & 0x0f;

                highByte += 0x30;

                if (highByte > 0x39)
                        dest[i * 2] = highByte + 0x07;
                else
                        dest[i * 2] = highByte;

                lowByte += 0x30;
                if (lowByte > 0x39)
                        dest[i * 2 + 1] = lowByte + 0x07;
                else
                        dest[i * 2 + 1] = lowByte;
        }
        return;
}
char *f = "%d";
int i = 1;
void dump_data(char *path, uint8_t *data, int32_t len)
{
        FILE *pf = NULL;
        char dest[1024 * 40] = {0};
        char fi[12] = {0};
        pf = fopen(path, "w+");
        if(pf != NULL)
        {
            fwrite(data, len, 1, pf);
            fclose(pf);            
        }
        else
        {
            printf("dump open file ERROR!\n");
            pf = fopen(path, "w+");
            if(pf != NULL)
            {
                fwrite(data, len, 1, pf);
                fclose(pf);            
            }
            else
            {
                printf("dump open file ERROR again!!!!!\n");
            }
        }
        

}

void load_data(char *path, uint8_t *data, int32_t *len)
{
    FILE *pf;
    int data_size = 0;
    pf = fopen(path, "r");
    if (!pf)
    {
            mprintf(0, "open image file error \n");
            return;
    }
    fseek(pf, 0, SEEK_END);
    data_size = ftell(pf);
    if(data == NULL)
    {
        *len = data_size;
        fclose(pf);
        return;        
    }
    fseek(pf, 0, SEEK_SET);
    fread(data, data_size, 1, pf);
    *len = data_size;
    fclose(pf);
    return;
}

bool load_conf(char *path, uint8_t *data, int32_t *len)
{
    FILE *pf;
    int data_size = 0;
    pf = fopen(path, "r");
    if (!pf)
    {
            mprintf(0, "open file error \n");
            return false;
    }
    mprintf(0, "open file success \n");
    fseek(pf, 0, SEEK_END);
    data_size = ftell(pf);
    if(data == NULL)
    {
        *len = data_size;
        fclose(pf);
        return true;        
    }
    fseek(pf, 0, SEEK_SET);
    fread(data, data_size, 1, pf);
    *len = data_size;
    fclose(pf);
    return true;
}

int seq = 1;
// void fbmp(uint8_t *img, int32_t w, int32_t h, char *name)
// {
// #ifdef M_BMP
//         char pth[1024] = {0};
//         ClImage *bmp = (ClImage *)malloc(sizeof(ClImage));
//         sprintf(pth + strlen(pth), "%d %s.bmp", seq, name);
// SAVE_BMP:
//         bmp->channels = 1;
//         bmp->width = w;
//         bmp->height = h;
//         bmp->imageData = (signed char *)img;

//         bool flag = clSaveImage(pth, bmp);
//         seq++;
// #endif
// }

// void fbmp_print(uint8_t *img, int32_t w, int32_t h, char *name, const char *cmd, ...)
// {
//         va_list args;
// #ifdef M_BMP
//         char pth[1024] = {0};
//         ClImage *bmp = (ClImage *)malloc(sizeof(ClImage));
//         sprintf(pth + strlen(pth), "%d %s.bmp", seq, name);
      
// SAVE_BMP:
//         bmp->channels = 1;
//         bmp->width = w;
//         bmp->height = h;
//         bmp->imageData = (signed char *)img;

//         bool flag = clSaveImage(pth, bmp);
//         seq++;

//         va_start(args, cmd);
//         vprintf(cmd, args);
//         va_end(args);
// #endif
// }

void print_array(unsigned char *buf, int len)
{
        for (int i = 0; i < len; i++)
                printf("0x%02X ", buf[i]);
        printf("\n");
}

void hex2str( uint8_t *in, uint32_t len, char *out, uint32_t *out_len )
{
    uint32_t i = 0;
    uint8_t tmp = 0;
    char *bak = out;

    if ( !in || !out || !out_len ) {
        return;
    }

    for ( i=0; i<len; i++ ) {
       tmp = ((*in)>>4)&0x0f;
       if ( tmp > 9 ) {
           *out++ = 'A' + tmp-0x0a;
       } else {
           *out++ = '0' + tmp;
       }
       tmp = (*in++)&0x0f;
       if ( tmp > 9 ) {
           *out++ = 'A' + tmp-0x0a;
       } else {
           *out++ = '0' + tmp;
       }
    }

    *out_len = out-bak;

}


void str2hex( char *in, uint32_t len, uint8_t *out, uint32_t *out_len )
{
    uint8_t low = 0;
    uint8_t high = 0;
    uint8_t *bak = out;
    char *end = in+len;

    if ( !in || !out || !out_len ) {
        return;
    }

    //LOG("len = %d\n", len);
    if ( len%2 != 0 ) {
        /*if ( len == 55 ) {*/
            /*dbg_backtrace();*/
        /*}*/
        /*LOG("len error\n");*/
        return;
    }
    while ( in < end ) {
        //LOG("in = %p\n", in);
        //LOG("end = %p\n", end);

        if ( (*in) >= 'A' ) {
            high = (*in++) - 'A' + 0x0A;
        } else {
            high = (*in++) - '0' + 0;
        }

        if ( (*in) >= 'A' ) {
            low = (*in++) - 'A' + 0x0A;
        } else {
            low = (*in++) - '0' + 0;
        }

        //LOG("called\n");
        *out++ = (high<<4)|low;
    }

    *out_len =  out - bak;
}

unsigned char print_time(char *buff)
{
    struct timeval tv;
    struct timezone tz;   
    struct tm *t;
    int i;
 
    gettimeofday(&tv, &tz);
    t = localtime(&tv.tv_sec);
    printf("time_now:%d-%d-%d %d:%d:%d.%ld\n", 1900+t->tm_year, 1+t->tm_mon, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, tv.tv_usec);
    i = sprintf(buff, "%02d:", t->tm_hour);   
    i += sprintf(buff + i, "%02d:", t->tm_min);   
    i += sprintf(buff + i, "%02d\n", t->tm_sec);   
}


void system_op(char * cmd_str)
{
    FILE *fp;
    unsigned char ret_buff[2048];
    fp = popen(cmd_str, "r");
    if(fp != NULL)
    {
        while(fgets(ret_buff, sizeof(ret_buff), fp) != NULL)
        {
            if('\n' == ret_buff[strlen(ret_buff)-1])
            {
                ret_buff[strlen(ret_buff)-1] = '\0';
            }
            printf("rm ./escode/upload.zip = %s\r\n", ret_buff);
        }
        pclose(fp);                   
    }
    
}