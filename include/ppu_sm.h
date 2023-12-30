#pragma once

#include <common.h>

#if defined(__cplusplus)
extern "C"
{
#endif

    void ppu_mode_oam(void);
    void ppu_mode_xfer(void);
    void ppu_mode_vblank(void);
    void ppu_mode_hblank(void);

#if defined(__cplusplus)
}
#endif
