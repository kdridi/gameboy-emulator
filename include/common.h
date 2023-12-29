#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

#define DEBUG_SCALE 4
#define DEBUG_SCREEN_WIDTH (16 * 8 * DEBUG_SCALE)
#define DEBUG_SCREEN_HEIGHT (32 * 8 * DEBUG_SCALE)

#define SERIAL_TRANSFER_DATA 0xFF01
#define SERIAL_TRANSFER_CONTROL 0xFF02

#define TIMER_DIVIDER 0xFF04
#define TIMER_COUNTER 0xFF05
#define TIMER_MODULO 0xFF06
#define TIMER_CONTROL 0xFF07

#define INTERRUPT_FLAG 0xFF0F
#define INTERRUPT_ENABLE 0xFFFF

#define DMA_TRANSFER 0xFF46

#define ADDR_VRAM_START 0x8000
#define ADDR_VRAM_END 0x9FFF

#define COLOR0 0xFF9BBC0F
#define COLOR1 0xFF8BAC0F
#define COLOR2 0xFF306230
#define COLOR3 0xFF0F380F

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define BIT(a, n) ((a >> n) & 1)

#define BIT_SET(a, n, on)   \
    do                      \
    {                       \
        if (on)             \
            a |= (1 << n);  \
        else                \
            a &= ~(1 << n); \
    } while (0)

#define BETWEEN(a, b, c) ((a >= b) && (a <= c))

#define NO_IMPL(...)                                                             \
    {                                                                            \
        do                                                                       \
        {                                                                        \
            fprintf(stderr, "NOT YET IMPLEMENTED: %s:%d\n", __FILE__, __LINE__); \
            abort();                                                             \
        } while (0);                                                             \
        return __VA_ARGS__;                                                      \
    }

#ifdef __cplusplus
extern "C"
{
#endif

    void delay(u32 ms);

#ifdef __cplusplus
}
#endif
