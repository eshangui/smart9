
/**
 * `b64.h' - b64
 *
 * copyright (c) 2014 joseph werle
 */

#ifndef B64_H
#define B64_H 1



int base64_encode(const unsigned char *in,  unsigned long len, 
                        unsigned char *out);
int base64_decode(const unsigned char *in, unsigned char *out);

#endif
