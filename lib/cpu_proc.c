#include <cpu.h>
#include <emu.h>
#include <bus.h>
#include <stack.h>

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

static void proc_ldh(cpu_context *ctx)
{
    if (ctx->dest_is_mem)
        bus_write(ctx->mem_dest, ctx->fetched_data & 0xFF);
    else
        cpu_write_reg(ctx->current_instruction->reg_1, bus_read(ctx->fetched_data & 0xFF00));

    emu_cycles(1);
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

static void proc_pop(cpu_context *ctx)
{
    u16 value = stack_pop16();
    emu_cycles(2);

    if (ctx->current_instruction->reg_1 == RT_AF)
        value &= 0xFFF0;

    cpu_write_reg(ctx->current_instruction->reg_1, value);
}

static void proc_push(cpu_context *ctx)
{
    u16 value = cpu_read_reg(ctx->current_instruction->reg_1);
    stack_push16(value);
    emu_cycles(2);
}

static void goto_addr(cpu_context *ctx, u16 addr, bool pushpc, bool poppc)
{
    if (check_cond(ctx))
    {
        if (pushpc)
        {
            stack_push16(ctx->regs.pc);
            emu_cycles(2);
        }

        if (poppc)
        {
            addr = stack_pop16();
            emu_cycles(2);
        }

        ctx->regs.pc = addr;
        emu_cycles(1);
    }
}

static void proc_jp(cpu_context *ctx)
{
    goto_addr(ctx, ctx->fetched_data, false, false);
}

static void proc_jr(cpu_context *ctx)
{
    char rel = (char)(ctx->fetched_data & 0xFF);
    u16 addr = ctx->regs.pc + rel;
    goto_addr(ctx, addr, false, false);
}

static void proc_call(cpu_context *ctx)
{
    goto_addr(ctx, ctx->fetched_data, true, false);
}

static void proc_rst(cpu_context *ctx)
{
    goto_addr(ctx, ctx->current_instruction->param, true, false);
}

static void proc_ret(cpu_context *ctx)
{
    if (ctx->current_instruction->cond != CT_NONE)
        emu_cycles(1);
    goto_addr(ctx, 0, false, true);
}

static void proc_reti(cpu_context *ctx)
{
    ctx->interrupts_enabled = true;
    proc_ret(ctx);
}

IN_PROC processors[] = {
    [IN_NONE] = proc_none,
    [IN_NOP] = proc_nop,
    [IN_LD] = proc_ld,
    [IN_JP] = proc_jp,
    [IN_JR] = proc_jr,
    [IN_CALL] = proc_call,
    [IN_RST] = proc_rst,
    [IN_RET] = proc_ret,
    [IN_RETI] = proc_reti,
    [IN_DI] = proc_di,
    [IN_XOR] = proc_xor,
    [IN_LDH] = proc_ldh,
    [IN_POP] = proc_pop,
    [IN_PUSH] = proc_push,
};

IN_PROC inst_get_processor(in_type type)
{
    return processors[type];
}
