#include "global.h"
#include "data_sync.h"

uint32_t data_sync(void)
{
    usleep(100000);
    mprintf(0,"sync: no data to sync\n");
    return D9_OK;
}
