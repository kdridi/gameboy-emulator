#pragma once

#include <common.h>
#include <cpu.h>

typedef enum
{
    IT_VBLANK = 0x01,   // This interrupt is requested when the PPU enters the V-Blank period.
    IT_LCD_STAT = 0x02, // This interrupt is requested when the STAT register enters an enabled state (ex. LYC == LY or Mode 0 -> 1)
    IT_TIMER = 0x04,    // This interrupt is requested when the TIMA register overflows (0xFF -> 0x00)
    IT_SERIAL = 0x08,   // This interrupt is requested when the serial transfer completes (after 8 clocks)
    IT_JOYPAD = 0x10    // This interrupt is requested when a button is pressed on the joypad.
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
