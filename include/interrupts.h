#pragma once

#include <common.h>
#include <cpu.h>

typedef enum
{
    IT_VBLANK = 0x01,
    IT_LCD_STAT = 0x02,
    IT_TIMER = 0x04,
    IT_SERIAL = 0x08,
    IT_JOYPAD = 0x10
} interrupt_type;

#ifdef __cplusplus
extern "C"
{
#endif

    void cpu_request_interrupt(interrupt_type type);
    void cpu_handle_interrupts(cpu_context *ctx);

#ifdef __cplusplus
}
#endif