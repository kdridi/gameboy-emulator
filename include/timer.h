#pragma once

#include <common.h>

typedef struct
{
    u16 div; // $FF04 - Divider Register (R/W)
    u8 tima; // $FF05 - Timer Counter (R/W)
    u8 tma;  // $FF06 - Timer Modulo (R/W)
    u8 tac;  // $FF07 - Timer Control (R/W)
} timer_context;

#ifdef __cplusplus
extern "C"
{
#endif

    void timer_init(void);
    void timer_tick(void);

    timer_context *timer_get_context(void);

    void timer_write(u16 address, u8 value);
    u8 timer_read(u16 address);

#ifdef __cplusplus
}
#endif
