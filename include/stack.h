#pragma once

#include <common.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void stack_push(u8 data);
    void stack_push16(u16 data);

    u8 stack_pop(void);
    u16 stack_pop16(void);

#ifdef __cplusplus
}
#endif
