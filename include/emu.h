#pragma once

#include <common.h>

#define EMU (emu_get_context())

typedef struct
{
    bool paused;
    bool running;
    bool die;
    u64 ticks;
} emu_context;

#ifdef __cplusplus
extern "C"
{
#endif

    void emu_init(void);

    int emu_run(int argc, char **argv);
    void emu_cycles(u64 cycles);

    emu_context *emu_get_context(void);

#ifdef __cplusplus
}
#endif
