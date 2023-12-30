#pragma once

#include <common.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void dma_start(u8 start);
    void dma_tick(void);

    bool dma_transfering(void);

#ifdef __cplusplus
}
#endif
