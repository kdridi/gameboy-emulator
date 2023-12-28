#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define BIT(a, n) ((a & (1 << n)) ? 1 : 0)

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
