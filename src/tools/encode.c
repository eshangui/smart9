#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timeb.h>
#include <sys/time.h>

static const char *codes = 
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
 
static const unsigned char map[256] = {
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 253, 255,
255, 253, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 253, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255,  62, 255, 255, 255,  63,
 52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255,
255, 254, 255, 255, 255,   0,   1,   2,   3,   4,   5,   6,
  7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,
 19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255,
255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,
 37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
 49,  50,  51, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255 };
 
int base64_encode(const unsigned char *in,  unsigned long len, 
                        unsigned char *out)
{
   unsigned long i, len2, leven;
   unsigned char *p;
   /* valid output size ? */
   len2 = 4 * ((len + 2) / 3);
   p = out;
   leven = 3*(len / 3);
   for (i = 0; i < leven; i += 3) {
       *p++ = codes[in[0] >> 2];
       *p++ = codes[((in[0] & 3) << 4) + (in[1] >> 4)];
       *p++ = codes[((in[1] & 0xf) << 2) + (in[2] >> 6)];
       *p++ = codes[in[2] & 0x3f];
       in += 3;
   }
   /* Pad it if necessary...  */
   if (i < len) {
       unsigned a = in[0];
       unsigned b = (i+1 < len) ? in[1] : 0;
       unsigned c = 0;
 
       *p++ = codes[a >> 2];
       *p++ = codes[((a & 3) << 4) + (b >> 4)];
       *p++ = (i+1 < len) ? codes[((b & 0xf) << 2) + (c >> 6)] : '=';
       *p++ = '=';
   }
 
   /* append a NULL byte */
   *p = '\0';
 
   return p - out;
}
 
int base64_decode(const unsigned char *in, unsigned char *out)
{
	unsigned long t, x, y, z;
	unsigned char c;
	int	g = 3;
 
	for (x = y = z = t = 0; in[x]!=0;) {
		c = map[in[x++]];
		if (c == 255) return -1;
		if (c == 253) continue;
		if (c == 254) { c = 0; g--; }
		t = (t<<6)|c;
		if (++y == 4) {
//			if (z + g > *outlen) { return CRYPT_BUFFER_OVERFLOW; }
			out[z++] = (unsigned char)((t>>16)&255);
			if (g > 1) out[z++] = (unsigned char)((t>>8)&255);
			if (g > 2) out[z++] = (unsigned char)(t&255);
			y = t = 0;
		}
	}
//	if (y != 0) {
//		return -1;
//	}
	return z;
}

unsigned int getstimeval()
{
    unsigned int us;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    us = (int)(tv.tv_usec);
    return us;
}


char* GF_GetGUID(char * buf)
{
    srand(getstimeval());
    const char *c = "89ab";
    //char buf[37];
    char *p = buf;
    int n;
    for( n = 0; n < 16; ++n )
    {
        int b = rand()%255;
        switch( n )
        {
        case 6:
            sprintf(p, "4%x", b%15 );
            break;
        case 8:
            sprintf(p, "%c%x", c[rand()%strlen(c)], b%15 );
            break;
        default:
            sprintf(p, "%02x", b);
            break;
        }

        p += 2;
        switch( n )
        {
        case 3:
        case 5:
        case 7:
        case 9:
            *p++ = '-';
            break;
        }
    }
    *p = 0;
    return buf;
}