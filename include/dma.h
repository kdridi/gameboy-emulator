#pragma once

#include <common.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void dma_start(u8 start);
    void dma_tick();

    bool dma_transfering();

#ifdef __cplusplus
}
#endif
