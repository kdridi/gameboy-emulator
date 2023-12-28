#include <cpu.h>
#include <emu.h>
#include <bus.h>
#include <stack.h>

static bool is_16bit(reg_type reg)
{
    switch (reg)
    {
    case RT_AF:
    case RT_BC:
    case RT_DE:
    case RT_HL:
    case RT_SP:
        return true;
    default:
        return false;
    }
}

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
        if (is_16bit(ctx->current_instruction->reg_1))
        {
            bus_write(ctx->mem_dest + 0, ctx->fetched_data & 0xFF);
            emu_cycles(1);
            bus_write(ctx->mem_dest + 1, ctx->fetched_data >> 8);
        }
        else
        {
            bus_write(ctx->mem_dest, ctx->fetched_data & 0xFF);
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
    cpu_set_flags(ctx->regs.a == 0, 0, 0, 0);
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

static void proc_inc(cpu_context *ctx)
{

    if (is_16bit(ctx->current_instruction->reg_1))
        emu_cycles(1);

    u16 value;
    if (ctx->dest_is_mem)
    {
        value = bus_read(ctx->mem_dest);
        value += 1;
        bus_write(ctx->mem_dest, value & 0xFF);
    }
    else
    {
        value = cpu_read_reg(ctx->current_instruction->reg_1);
        value += 1;
        cpu_write_reg(ctx->current_instruction->reg_1, value);
    }

    if ((ctx->current_opcode & 0x03) == 0x03)
        return;

    cpu_set_flags(value == 0, 0, (value & 0x0F) == 0, -1);
}

static void proc_dec(cpu_context *ctx)
{

    if (is_16bit(ctx->current_instruction->reg_1))
        emu_cycles(1);

    u16 value;
    if (ctx->dest_is_mem)
    {
        value = bus_read(ctx->mem_dest);
        value -= 1;
        bus_write(ctx->mem_dest, value & 0xFF);
    }
    else
    {
        value = cpu_read_reg(ctx->current_instruction->reg_1);
        value -= 1;
        cpu_write_reg(ctx->current_instruction->reg_1, value);
    }

    if ((ctx->current_opcode & 0x0B) == 0x0B)
        return;

    cpu_set_flags(value == 0, 1, (value & 0x0F) == 0x0F, -1);
}

static void proc_adc(cpu_context *ctx)
{
    u16 u = ctx->fetched_data;
    u16 a = ctx->regs.a;
    u16 c = CPU_FLAG_C(ctx);

    u16 value = a + u + c;
    ctx->regs.a = value & 0xFF;

    cpu_set_flags(ctx->regs.a == 0, 0, (a & 0x0F) + (u & 0x0F) + c > 0x0F, value > 0xFF);
}

static void proc_add(cpu_context *ctx)
{
    u32 value = cpu_read_reg(ctx->current_instruction->reg_1) + ctx->fetched_data;

    bool is16 = is_16bit(ctx->current_instruction->reg_1);
    if (is16)
        emu_cycles(1);

    if (ctx->current_instruction->reg_1 == RT_SP)
        value = cpu_read_reg(RT_SP) + (int8_t)ctx->fetched_data;

    u8 zflag = (value & 0xFF) == 0;
    u8 hflag = (cpu_read_reg(ctx->current_instruction->reg_1) & 0x0F) + (ctx->fetched_data & 0x0F) > 0x0F;
    u8 cflag = (int16_t)(cpu_read_reg(ctx->current_instruction->reg_1) & 0xFF) + (int16_t)(ctx->fetched_data & 0xFF) > 0xFF;

    if (is16)
    {
        zflag = -1;
        hflag = (cpu_read_reg(ctx->current_instruction->reg_1) & 0x0FFF) + (ctx->fetched_data & 0x0FFF) > 0x0FFF;
        cflag = (int32_t)(cpu_read_reg(ctx->current_instruction->reg_1) & 0xFFFF) + (int32_t)(ctx->fetched_data & 0xFFFF) > 0xFFFF;
    }

    if (ctx->current_instruction->reg_1 == RT_SP)
    {
        zflag = 0;
        hflag = (cpu_read_reg(ctx->current_instruction->reg_1) & 0x0F) + (ctx->fetched_data & 0x0F) > 0x0F;
        cflag = (int16_t)(cpu_read_reg(ctx->current_instruction->reg_1) & 0xFF) + (int16_t)(ctx->fetched_data & 0xFF) > 0xFF;
    }

    cpu_write_reg(ctx->current_instruction->reg_1, value & 0xFFFF);
    cpu_set_flags(zflag, 0, hflag, cflag);
}

static void proc_sub(cpu_context *ctx)
{
    u16 value = cpu_read_reg(ctx->current_instruction->reg_1) - ctx->fetched_data;

    u8 zflag = (value & 0xFF) == 0;
    u8 hflag = (cpu_read_reg(ctx->current_instruction->reg_1) & 0x0F) < (ctx->fetched_data & 0x0F);
    u8 cflag = (cpu_read_reg(ctx->current_instruction->reg_1) & 0xFF) < (ctx->fetched_data & 0xFF);

    cpu_write_reg(ctx->current_instruction->reg_1, value & 0xFFFF);
    cpu_set_flags(zflag, 1, hflag, cflag);
}

static void proc_sbc(cpu_context *ctx)
{
    u16 u = ctx->fetched_data;
    u16 a = cpu_read_reg(ctx->current_instruction->reg_1);
    u16 c = CPU_FLAG_C(ctx);

    u16 value = a - u - c;
    cpu_write_reg(ctx->current_instruction->reg_1, value & 0xFFFF);

    u8 zflag = (value & 0xFF) == 0;
    u8 hflag = (a & 0x0F) < (u & 0x0F) + c;
    u8 cflag = (a & 0xFF) < (u & 0xFF) + c;

    cpu_set_flags(zflag, 1, hflag, cflag);
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
    [IN_INC] = proc_inc,
    [IN_DEC] = proc_dec,
    [IN_ADC] = proc_adc,
    [IN_ADD] = proc_add,
    [IN_SUB] = proc_sub,
    [IN_SBC] = proc_sbc,
};

IN_PROC inst_get_processor(in_type type)
{
    return processors[type];
}
