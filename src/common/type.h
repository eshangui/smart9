#ifndef _TYPE_H_
#define _TYPE_H_

#define _EMBEDDED_	1
typedef unsigned char uint8_t;

typedef unsigned int uint32_t;
#ifndef __cplusplus
//typedef int bool;
#define true 1
#define false 0
#endif
typedef char char_t;
typedef unsigned short uint16_t;
typedef int int32_t;
#ifndef __linux
typedef signed char int8_t;
typedef short int16_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;
#else
#include "stdint.h"
#endif

#endif