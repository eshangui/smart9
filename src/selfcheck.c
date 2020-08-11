#include "global.h"
#include "selfcheck.h"

uint32_t selfcheck(void)
{
    usleep(100000);
    mprintf(0,"selfcheck prt ok\n");
    usleep(100000);
    mprintf(0,"selfcheck se ok\n");
    usleep(100000);
    mprintf(0,"selfcheck net ok\n");
    return D9_OK;
}