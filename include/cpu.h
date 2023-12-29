#pragma once

#include <common.h>
#include <instruction.h>

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

    bool int_master_enabled;
    bool enabling_ime;
    u8 ie_register;
    u8 int_flags;

} cpu_context;

typedef void (*IN_PROC)(cpu_context *);

#ifdef __cplusplus
extern "C"
{
#endif

    void cpu_init(void);
    bool cpu_step(void);

    u16 cpu_read_reg(reg_type rt);
    void cpu_write_reg(reg_type rt, u16 value);

    u8 cpu_read_reg8(reg_type rt);
    void cpu_write_reg8(reg_type rt, u8 value);

    void cpu_fetch_data(void);

    void cpu_set_flags(u8 z, u8 n, u8 h, u8 c);

    u8 cpu_get_ie_register();
    void cpu_set_ie_register(u8 n);

    u8 cpu_get_int_flags();
    void cpu_set_int_flags(u8 value);

    IN_PROC inst_get_processor(in_type type);

    cpu_registers *cpu_get_registers(void);

    void instr_to_str(cpu_context *ctx, char *str);

#ifdef __cplusplus
}
#endif

#define CPU_FLAG_Z(ctx) BIT(ctx->regs.f, 7)
#define CPU_FLAG_N(ctx) BIT(ctx->regs.f, 6)
#define CPU_FLAG_H(ctx) BIT(ctx->regs.f, 5)
#define CPU_FLAG_C(ctx) BIT(ctx->regs.f, 4)