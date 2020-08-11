#include <stdio.h>
#include <stdarg.h>
#include "type.h"
#include "var.h"
#include "bmp.h"
#include "dbg.h"
#if _WIN32
#include <stdint.h>
#else
#include <unistd.h>
#endif
#include <sys/timeb.h>

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
        FILE *pf;
        char dest[1024 * 40] = {0};
        char fi[12] = {0};
        pf = fopen(path, "w+");
        fwrite(data, len, 1, pf);
        fclose(pf);
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
        fseek(pf, 0, SEEK_SET);
        fread(data, data_size, 1, pf);
        *len = data_size;
        fclose(pf);
        return;
}

int seq = 1;
void fbmp(uint8_t *img, int32_t w, int32_t h, char *name)
{
#ifdef M_BMP
        char pth[1024] = {0};
        ClImage *bmp = (ClImage *)malloc(sizeof(ClImage));
        sprintf(pth + strlen(pth), "%d %s.bmp", seq, name);
SAVE_BMP:
        bmp->channels = 1;
        bmp->width = w;
        bmp->height = h;
        bmp->imageData = (signed char *)img;

        bool flag = clSaveImage(pth, bmp);
        seq++;
#endif
}

void fbmp_print(uint8_t *img, int32_t w, int32_t h, char *name, const char *cmd, ...)
{
        va_list args;
#ifdef M_BMP
        char pth[1024] = {0};
        ClImage *bmp = (ClImage *)malloc(sizeof(ClImage));
        sprintf(pth + strlen(pth), "%d %s.bmp", seq, name);
      
SAVE_BMP:
        bmp->channels = 1;
        bmp->width = w;
        bmp->height = h;
        bmp->imageData = (signed char *)img;

        bool flag = clSaveImage(pth, bmp);
        seq++;

        va_start(args, cmd);
        vprintf(cmd, args);
        va_end(args);
#endif
}

void print_array(unsigned char *buf, int len)
{
        for (int i = 0; i < len; i++)
                printf("0x%x ", buf[i]);
        printf("\n");
}
