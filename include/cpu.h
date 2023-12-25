#pragma once

#include <common.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void cpu_init(void);
    bool cpu_step(void);

#ifdef __cplusplus
}
#endif