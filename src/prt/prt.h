


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
	uint8_t rgbBlue; //该颜色的蓝色分量
	uint8_t rgbGreen; //该颜色的绿色分量
	uint8_t rgbRed; //该颜色的红色分量
	uint8_t rgbReserved; //保留值
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

static const char *ESCPOS_CMD_INIT = "\x1b\x40";
static const char *ESCPOS_CMD_PRINT_RASTER_BIT_IMAGE = "\x1d\x76\x30\x00";
static const char *ESCPOS_CMD_CUT = "\x1d\x56\x42";
static const char *ESCPOS_CMD_FEED = "\x1b\x64";

// The maximum width of image the printer can accept
static const int ESCPOS_MAX_DOT_WIDTH = 576;

// When printing, if the image is too long, the printer
// will cut the images into chunks of (w x ESCPOS_CHUNK_DOT_HEIGHT)
static const int ESCPOS_CHUNK_DOT_HEIGHT = 512;

void prt_connect(void);
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
