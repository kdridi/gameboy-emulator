#pragma once

#include <common.h>
#include <instructions.h>

typedef struct
{
    u8 a, f, b, c, d, e, h, l;
    u16 pc, sp;
} cpu_registers;

typedef struct
{
    cpu_registers regs;

    u16 fetched_data;
    u16 mem_dest;
    bool dest_is_mem;
    u8 current_opcode;
    instruction *current_instruction;

    bool halted;
    bool stepping;
} cpu_context;

#ifdef __cplusplus
extern "C"
{
#endif

    void cpu_init(void);
    bool cpu_step(void);

#ifdef __cplusplus
}
#endif