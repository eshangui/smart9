#ifndef _PRT_H_
#define _PRT_H_
#include "mongoose.h"
#include "esc2bmp.h"

extern esc2bmp_handle_t prt_handle;


typedef struct escpos_config {
    // See ESCPOS_MAX_DOT_WIDTH in constants.h
    // Default value: ESCPOS_MAX_DOT_WIDTH
    int max_width;

    // See ESCPOS_CHUNK_DOT_HEIGHT in constants.h
    // Default value: ESCPOS_CHUNK_DOT_HEIGHT
    int chunk_height;

    unsigned int is_network_printer : 1;
} escpos_config;

typedef struct escpos_printer {
    int sockfd;
    escpos_config config;
} escpos_printer;

typedef struct
{
	//unsigned short    bfType;
	uint32_t   bfSize;
	uint16_t    bfReserved1;
	uint16_t    bfReserved2;
	uint32_t   bfOffBits;
} ClBitMapFileHeader;
 
typedef struct
{
	uint32_t biSize; 
	uint32_t   biWidth; 
	uint32_t   biHeight; 
	uint16_t   biPlanes; 
	uint16_t   biBitCount;
	uint32_t   biCompression; 
	uint32_t   biSizeImage; 
	uint32_t    biXPelsPerMeter; 
	uint32_t    biYPelsPerMeter; 
	uint32_t    biClrUsed; 
	uint32_t    biClrImportant; 
} ClBitMapInfoHeader;
 
typedef struct 
{
	uint8_t rgbBlue; //该�?�色的蓝色分�?
	uint8_t rgbGreen; //该�?�色的绿色分�?
	uint8_t rgbRed; //该�?�色的红色分�?
	uint8_t rgbReserved; //保留�?
} ClRgbQuad;
 
typedef struct
{
	int32_t width;
	int32_t height;
	int32_t channels;
	int8_t* imageData;
}ClImage;
 
ClImage* clLoadImage(char* path);
bool clSaveImage(char* path, ClImage* bmpImg);

static const unsigned char ESCPOS_CMD_INIT[] = {0x1b, 0x40};
//static const char *ESCPOS_CMD_PRINT_RASTER_BIT_IMAGE = "\x1d\x76\x30\x00";
static const unsigned char ESCPOS_CMD_CUT0[] = {0x1d, 0x56, 0x0};
static const unsigned char ESCPOS_CMD_CUT1[] = {0x1d, 0x56, 0x1};
static const unsigned char ESCPOS_CMD_CUT2[] = {0x1d, 0x56, 0x42, 0x00}; //static const char *ESCPOS_CMD_CUT2 = "\x1d\x56\x42";
static const unsigned char ESCPOS_CMD_CUT3[] = {0x1d, 0x0c}; // unknown command by PAWOON, assume it to be CUT cmd
static const unsigned char ESCPOS_CMD_FEED[] = {0x1b, 0x64};
static const unsigned char ESCPOS_CMD_CASHBOX[] = {0x1b, 0x70};
static const unsigned char ESCPOS_CMD_CUT_0[] = {0x1b, 0x69};
static const unsigned char ESCPOS_CMD_CUT_1[] = {0x1b, 0x6D};
static const char *ESCPOS_CMD_ALIGN_CENTER = "\x1b\x61\x01";

// The maximum width of image the printer can accept
static const int ESCPOS_MAX_DOT_WIDTH = 576;

// When printing, if the image is too long, the printer
// will cut the images into chunks of (w x ESCPOS_CHUNK_DOT_HEIGHT)
static const int ESCPOS_CHUNK_DOT_HEIGHT = 512;

void prt_init(void);
void prt_print (unsigned char* data, int len);

int escpos_printer_cut(const int lines);
int escpos_printer_feed( const int lines);
unsigned char *load_image(const char * const image_path,
                          int *width,
                          int *height);
        
int escpos_printer_image(escpos_printer *printer,
                         const unsigned char * const image_data,
                         const int width,
                         const int height);

void updata_offine_json(char *json_str, char *code_id);
unsigned char get_offline_code(void);

int print_end_string(void *buf);


#endif
