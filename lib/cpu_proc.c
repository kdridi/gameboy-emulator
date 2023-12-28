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
    ctx->int_master_enabled = false;
}

static void proc_ei(cpu_context *ctx)
{
    ctx->enabling_ime = true;
}

static void proc_ldh(cpu_context *ctx)
{
    if (ctx->current_instruction->reg_1 == RT_A)
        cpu_write_reg(ctx->current_instruction->reg_1, bus_read(0xFF00 | ctx->fetched_data));
    else
        bus_write(ctx->mem_dest, ctx->regs.a);
    emu_cycles(1);
}

static void proc_ld(cpu_context *ctx)
{
    if (ctx->dest_is_mem)
    {
        if (is_16bit(ctx->current_instruction->reg_2))
        {
            bus_write16(ctx->mem_dest, ctx->fetched_data);
            emu_cycles(2);
            return;
        }

        bus_write(ctx->mem_dest, ctx->fetched_data & 0xFF);
        emu_cycles(1);
        return;
    }

    if (ctx->current_instruction->mode == AM_HL_SPR)
    {
        u8 hflag = (cpu_read_reg(ctx->current_instruction->reg_2) & 0x00F) + (ctx->fetched_data & 0x00F) > 0x00F;
        u8 cflag = (cpu_read_reg(ctx->current_instruction->reg_2) & 0x0FF) + (ctx->fetched_data & 0x0FF) > 0x0FF;

        cpu_set_flags(0, 0, hflag, cflag);
        cpu_write_reg(ctx->current_instruction->reg_1, cpu_read_reg(ctx->current_instruction->reg_2) + (char)ctx->fetched_data);

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

    emu_cycles(1);
}

static void goto_addr(cpu_context *ctx, u16 addr, bool pushpc)
{
    if (check_cond(ctx))
    {
        if (pushpc)
        {
            stack_push16(ctx->regs.pc);
            emu_cycles(2);
        }

        ctx->regs.pc = addr;
        emu_cycles(1);
    }
}

static void proc_ret(cpu_context *ctx)
{
    if (ctx->current_instruction->cond != CT_NONE)
        emu_cycles(1);

    if (check_cond(ctx))
    {
        u16 addr = stack_pop16();
        emu_cycles(2);

        ctx->regs.pc = addr;
        emu_cycles(1);
    }
}

static void proc_reti(cpu_context *ctx)
{
    ctx->int_master_enabled = true;
    proc_ret(ctx);
}

static void proc_jp(cpu_context *ctx)
{
    goto_addr(ctx, ctx->fetched_data, false);
}

static void proc_jr(cpu_context *ctx)
{
    int8_t rel = (int8_t)(ctx->fetched_data & 0xFF);
    u16 addr = ctx->regs.pc + rel;
    goto_addr(ctx, addr, false);
}

static void proc_call(cpu_context *ctx)
{
    goto_addr(ctx, ctx->fetched_data, true);
}

static void proc_rst(cpu_context *ctx)
{
    goto_addr(ctx, ctx->current_instruction->param, true);
}

static void proc_inc(cpu_context *ctx)
{
    u16 val = cpu_read_reg(ctx->current_instruction->reg_1) + 1;

    if (is_16bit(ctx->current_instruction->reg_1))
    {
        emu_cycles(1);
    }

    if (ctx->current_instruction->reg_1 == RT_HL && ctx->current_instruction->mode == AM_MR)
    {
        val = bus_read(cpu_read_reg(RT_HL)) + 1;
        val &= 0xFF;
        bus_write(cpu_read_reg(RT_HL), val);
    }
    else
    {
        cpu_write_reg(ctx->current_instruction->reg_1, val);
        val = cpu_read_reg(ctx->current_instruction->reg_1);
    }

    if ((ctx->current_opcode & 0x03) == 0x03)
    {
        return;
    }

    cpu_set_flags(val == 0, 0, (val & 0x0F) == 0, -1);
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
        value = cpu_read_reg(ctx->current_instruction->reg_1) + (int8_t)ctx->fetched_data;

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
    u8 value = ctx->regs.a - ctx->fetched_data;

    u8 zflag = (value & 0xFF) == 0;
    u8 hflag = (ctx->regs.a & 0x0F) < (ctx->fetched_data & 0x0F);
    u8 cflag = (ctx->regs.a & 0xFF) < (ctx->fetched_data & 0xFF);

    ctx->regs.a = value & 0xFF;
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

static void proc_and(cpu_context *ctx)
{
    ctx->regs.a &= ctx->fetched_data & 0xFF;
    cpu_set_flags(ctx->regs.a == 0, 0, 1, 0);
}

static void proc_or(cpu_context *ctx)
{
    assert(ctx->current_instruction->reg_1 == RT_A);

    ctx->regs.a |= ctx->fetched_data & 0xFF;
    cpu_set_flags(ctx->regs.a == 0, 0, 0, 0);
}

static void proc_cp(cpu_context *ctx)
{
    assert(ctx->current_instruction->reg_1 == RT_A);

    int16_t value = (int16_t)ctx->regs.a - (int16_t)ctx->fetched_data;

    u8 zflag = (value & 0xFF) == 0;
    u8 hflag = (int16_t)(ctx->regs.a & 0x0F) < (int16_t)(ctx->fetched_data & 0x0F);
    u8 cflag = (int16_t)(ctx->regs.a & 0xFF) < (int16_t)(ctx->fetched_data & 0xFF);

    cpu_set_flags(zflag, 1, hflag, cflag);
}

reg_type rt_lookup[] = {
    [0x00] = RT_B,
    [0x01] = RT_C,
    [0x02] = RT_D,
    [0x03] = RT_E,
    [0x04] = RT_H,
    [0x05] = RT_L,
    [0x06] = RT_HL,
    [0x07] = RT_A,
};

reg_type decode_reg(u8 reg)
{
    if (reg > 0b111)
        return RT_NONE;
    return rt_lookup[reg];
}

static void proc_cb(cpu_context *ctx)
{
    u8 op = ctx->fetched_data;             // op = 0x40 = 0b 01 000 000
    reg_type reg = decode_reg(op & 0b111); // reg = RT_B
    u8 bit = (op >> 3) & 0b111;            // bit = 0
    u8 bit_op = (op >> 6) & 0b11;          // bit_op = 1
    u8 reg_val = cpu_read_reg8(reg);       // reg_val = 0x00

    emu_cycles(1);

    if (reg == RT_HL)
        emu_cycles(2);

    switch (bit_op)
    {
    case 1: // BIT
        cpu_set_flags(!BIT(reg_val, bit), 0, 1, -1);
        return;
    case 2: // RESET
        reg_val &= ~(1 << bit);
        cpu_write_reg8(reg, reg_val);
        return;
    case 3: // SET
        reg_val |= (1 << bit);
        cpu_write_reg8(reg, reg_val);
        return;
    }

    bool flagC = CPU_FLAG_C(ctx);

    switch (bit)
    {
    case 0: // RLC
    {
        u8 old = reg_val;
        reg_val <<= 1;
        reg_val |= BIT(old, 7);

        cpu_write_reg8(reg, reg_val);
        cpu_set_flags(reg_val == 0, 0, 0, BIT(old, 7));
        return;
    }
    case 1: // RRC
    {
        u8 old = reg_val;
        reg_val >>= 1;
        reg_val |= BIT(old, 0) << 7;

        cpu_write_reg8(reg, reg_val);
        cpu_set_flags(reg_val == 0, 0, 0, BIT(old, 0));
        return;
    }
    case 2: // RL
    {
        u8 old = reg_val;
        reg_val <<= 1;
        reg_val |= flagC;

        cpu_write_reg8(reg, reg_val);
        cpu_set_flags(reg_val == 0, 0, 0, BIT(old, 7));
        return;
    }
    case 3: // RR
    {
        u8 old = reg_val;
        reg_val >>= 1;
        reg_val |= flagC << 7;

        cpu_write_reg8(reg, reg_val);
        cpu_set_flags(reg_val == 0, 0, 0, BIT(old, 0));
        return;
    }
    case 4: // SLA
    {
        u8 old = reg_val;
        reg_val <<= 1;

        cpu_write_reg8(reg, reg_val);
        cpu_set_flags(reg_val == 0, 0, 0, BIT(old, 7));
        return;
    }
    case 5: // SRA
    {
        u8 old = reg_val;
        reg_val >>= 1;
        reg_val |= old & 0x80;

        cpu_write_reg8(reg, reg_val);
        cpu_set_flags(reg_val == 0, 0, 0, BIT(old, 0));
        return;
    }
    case 6: // SWAP
    {
        reg_val = ((reg_val & 0x0F) << 4) | ((reg_val & 0xF0) >> 4);

        cpu_write_reg8(reg, reg_val);
        cpu_set_flags(reg_val == 0, 0, 0, 0);
        return;
    }
    case 7: // SRL
    {
        u8 old = reg_val;
        reg_val >>= 1;

        cpu_write_reg8(reg, reg_val);
        cpu_set_flags(reg_val == 0, 0, 0, BIT(old, 0));
        return;
    }
    }

    NO_IMPL();
}

static void proc_rlca(cpu_context *ctx)
{
    u8 u = ctx->regs.a;
    bool c = BIT(u, 7);
    u <<= 1;
    u |= c;
    ctx->regs.a = u;

    cpu_set_flags(0, 0, 0, c);
}

static void proc_rrca(cpu_context *ctx)
{
    u8 u = ctx->regs.a;
    bool c = BIT(u, 0);
    u >>= 1;
    u |= c << 7;
    ctx->regs.a = u;

    cpu_set_flags(0, 0, 0, c);
}

static void proc_rla(cpu_context *ctx)
{
    u8 u = ctx->regs.a;
    bool c = BIT(u, 7);
    u <<= 1;
    u |= CPU_FLAG_C(ctx);
    ctx->regs.a = u;

    cpu_set_flags(0, 0, 0, c);
}

static void proc_rra(cpu_context *ctx)
{
    u8 u = ctx->regs.a;
    bool c = BIT(u, 0);
    u >>= 1;
    u |= CPU_FLAG_C(ctx) << 7;
    ctx->regs.a = u;

    cpu_set_flags(0, 0, 0, c);
}

static void proc_stop(cpu_context *ctx)
{
    ctx->halted = true;
    // NO_IMPL();
}

static void proc_daa(cpu_context *ctx)
{
    u8 u = 0;
    int fc = 0;

    if (CPU_FLAG_H(ctx) || (!CPU_FLAG_N(ctx) && (ctx->regs.a & 0xF) > 9))
        u = 6;

    if (CPU_FLAG_C(ctx) || (!CPU_FLAG_N(ctx) && ctx->regs.a > 0x99))
    {
        u |= 0x60;
        fc = 1;
    }

    ctx->regs.a += CPU_FLAG_N(ctx) ? -u : u;

    cpu_set_flags(ctx->regs.a == 0, -1, 0, fc);
}

static void proc_cpl(cpu_context *ctx)
{
    ctx->regs.a = ~ctx->regs.a;
    cpu_set_flags(-1, 1, 1, -1);
}

static void proc_scf(cpu_context *ctx)
{
    cpu_set_flags(-1, 0, 0, 1);
}

static void proc_ccf(cpu_context *ctx)
{
    cpu_set_flags(-1, 0, 0, !CPU_FLAG_C(ctx));
}

static void proc_halt(cpu_context *ctx)
{
    ctx->halted = true;
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
    [IN_EI] = proc_ei,
    [IN_XOR] = proc_xor,
    [IN_AND] = proc_and,
    [IN_OR] = proc_or,
    [IN_CP] = proc_cp,
    [IN_LDH] = proc_ldh,
    [IN_POP] = proc_pop,
    [IN_PUSH] = proc_push,
    [IN_INC] = proc_inc,
    [IN_DEC] = proc_dec,
    [IN_ADC] = proc_adc,
    [IN_ADD] = proc_add,
    [IN_SUB] = proc_sub,
    [IN_SBC] = proc_sbc,
    [IN_CB] = proc_cb,
    [IN_RLCA] = proc_rlca,
    [IN_RRCA] = proc_rrca,
    [IN_RLA] = proc_rla,
    [IN_RRA] = proc_rra,
    [IN_STOP] = proc_stop,
    [IN_DAA] = proc_daa,
    [IN_CPL] = proc_cpl,
    [IN_SCF] = proc_scf,
    [IN_CCF] = proc_ccf,
    [IN_HALT] = proc_halt,
};

IN_PROC inst_get_processor(in_type type)
{
    return processors[type];
}
