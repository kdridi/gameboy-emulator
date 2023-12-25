#pragma once

#include <common.h>

typedef struct
{
    bool paused;
    bool running;
    u64 ticks;
} emu_context;

#ifdef __cplusplus
extern "C"
{
#endif

    int emu_run(int argc, char **argv);
    void emu_cycles(u64 cycles);

    emu_context *emu_get_context(void);

#ifdef __cplusplus
}
#endif