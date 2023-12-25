#include <cpu.h>
#include <emu.h>

static void proc_none(cpu_context *ctx)
{
    printf("INVALID INSTRUCTION: 0x%02X\n", ctx->current_opcode);
    abort();
}

static void proc_nop(cpu_context *ctx)
{
}

static void proc_di(cpu_context *ctx)
{
    ctx->interrupts_enabled = false;
}

static void proc_ld(cpu_context *ctx)
{
    // TODO: Implement
}

static void proc_xor(cpu_context *ctx)
{
    ctx->regs.a ^= ctx->fetched_data & 0xFF;
    cpu_set_flags(ctx->regs.a, 0, 0, 0);
}

static bool check_cond(cpu_context *ctx)
{
    bool z = CPU_FLAG_Z(ctx);
    bool c = CPU_FLAG_C(ctx);

    // clang-format off
    switch (ctx->current_instruction->cond)
    {
    case CT_NONE: return true;
    case CT_NZ: return !z;
    case CT_Z: return z;
    case CT_NC: return !c;
    case CT_C: return c;
    }
    // clang-format on

    return false;
}

static void proc_jp(cpu_context *ctx)
{
    if (check_cond(ctx))
    {
        ctx->regs.pc = ctx->fetched_data;
        emu_cycles(1);
    }
}

IN_PROC processors[] = {
    [IN_NONE] = proc_none,
    [IN_NOP] = proc_nop,
    [IN_LD] = proc_ld,
    [IN_JP] = proc_jp,
    [IN_DI] = proc_di,
    [IN_XOR] = proc_xor,
};

IN_PROC inst_get_processor(in_type type)
{
    return processors[type];
}
