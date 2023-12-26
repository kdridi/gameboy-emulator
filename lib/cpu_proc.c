#include <cpu.h>
#include <emu.h>
#include <bus.h>

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
    if (ctx->dest_is_mem)
    {
        switch (ctx->current_instruction->reg_2)
        {
        case RT_AF:
        case RT_BC:
        case RT_DE:
        case RT_HL:
        case RT_SP:
            bus_write(ctx->mem_dest + 0, ctx->fetched_data & 0xFF);
            emu_cycles(1);
            bus_write(ctx->mem_dest + 1, ctx->fetched_data >> 8);
            break;
        default:
            bus_write(ctx->mem_dest, ctx->fetched_data & 0xFF);
            break;
        }

        return;
    }

    if (ctx->current_instruction->mode == AM_HL_SPR)
    {
        u8 hflag = (cpu_read_reg(ctx->current_instruction->reg_2) & 0x0F) + (ctx->fetched_data & 0xF0) > 0x0F;
        u8 cflag = (cpu_read_reg(ctx->current_instruction->reg_2) & 0xFF) + (ctx->fetched_data & 0xFF) > 0xFF;

        cpu_set_flags(0, 0, hflag, cflag);

        cpu_write_reg(ctx->current_instruction->reg_1, cpu_read_reg(ctx->current_instruction->reg_2) + (int8_t)ctx->fetched_data);

        return;
    }

    cpu_write_reg(ctx->current_instruction->reg_1, ctx->fetched_data);
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
