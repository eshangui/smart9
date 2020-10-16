#include "global.h"
#include "db.h"
#include <string.h>

uint32_t get_prt_sn(char* sn)
{
    const char* tmp = "0000000000000001";
    strcpy(sn, tmp);
}