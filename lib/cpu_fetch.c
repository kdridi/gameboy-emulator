#include <cpu.h>
#include <bus.h>
#include <emu.h>

extern cpu_context ctx;

void cpu_fetch_data(void)
{
    ctx.mem_dest = 0;
    ctx.dest_is_mem = false;

    if (ctx.current_instruction == NULL)
        return;

    switch (ctx.current_instruction->mode)
    {
    case AM_NONE:
        assert(ctx.current_instruction->reg_1 == RT_NONE);
        assert(ctx.current_instruction->reg_2 == RT_NONE);
        return;

    case AM_R:
        assert(ctx.current_instruction->reg_1 != RT_NONE);
        assert(ctx.current_instruction->reg_2 == RT_NONE);
        assert(ctx.current_instruction->cond == CT_NONE);

        ctx.fetched_data = cpu_read_reg(ctx.current_instruction->reg_1);
        return;

    case AM_R_R:
        assert(ctx.current_instruction->reg_1 != RT_NONE);
        assert(ctx.current_instruction->reg_2 != RT_NONE);
        assert(ctx.current_instruction->cond == CT_NONE);

        ctx.fetched_data = cpu_read_reg(ctx.current_instruction->reg_2);
        return;

    case AM_D16:
        assert(ctx.current_instruction->reg_1 == RT_NONE);
    case AM_R_D16:
    case AM_R_A16:
    {
        assert(ctx.current_instruction->reg_2 == RT_NONE);
        assert(ctx.current_instruction->mode == AM_D16 || ctx.current_instruction->cond == CT_NONE);

        u16 lo = bus_read(ctx.regs.pc);
        emu_cycles(1);
        ctx.regs.pc++;

        u16 hi = bus_read(ctx.regs.pc);
        emu_cycles(1);
        ctx.regs.pc++;

        ctx.fetched_data = (hi << 8) | lo;
        if (ctx.current_instruction->mode == AM_R_A16)
        {
            ctx.fetched_data = bus_read(ctx.fetched_data);
            emu_cycles(1);
        }

        return;
    }

    case AM_MR_R:
    {
        assert(ctx.current_instruction->reg_1 != RT_NONE);
        assert(ctx.current_instruction->reg_2 != RT_NONE);
        assert(ctx.current_instruction->cond == CT_NONE);

        ctx.fetched_data = cpu_read_reg(ctx.current_instruction->reg_2);
        ctx.mem_dest = cpu_read_reg(ctx.current_instruction->reg_1);
        ctx.dest_is_mem = true;

        if (ctx.current_instruction->reg_1 == RT_C)
            ctx.mem_dest |= 0xFF00;

        return;
    }

    case AM_MR:
    {
        assert(ctx.current_instruction->reg_1 != RT_NONE);
        assert(ctx.current_instruction->reg_2 == RT_NONE);
        assert(ctx.current_instruction->cond == CT_NONE);

        ctx.mem_dest = cpu_read_reg(ctx.current_instruction->reg_1);
        ctx.dest_is_mem = true;
        ctx.fetched_data = bus_read(cpu_read_reg(ctx.current_instruction->reg_1));
        emu_cycles(1);

        return;
    }

    case AM_R_MR:
    {
        assert(ctx.current_instruction->reg_1 != RT_NONE);
        assert(ctx.current_instruction->reg_2 != RT_NONE);
        assert(ctx.current_instruction->cond == CT_NONE);

        u16 addr = cpu_read_reg(ctx.current_instruction->reg_2);
        if (ctx.current_instruction->reg_2 == RT_C)
            addr |= 0xFF00;

        ctx.fetched_data = bus_read(addr);
        emu_cycles(1);

        return;
    }

    case AM_R_RI:
    {
        assert(ctx.current_instruction->reg_1 != RT_NONE);
        assert(ctx.current_instruction->reg_2 != RT_NONE);
        assert(ctx.current_instruction->cond == CT_NONE);

        u16 addr = cpu_read_reg(ctx.current_instruction->reg_2);
        ctx.fetched_data = bus_read(addr);
        emu_cycles(1);
        cpu_write_reg(ctx.current_instruction->reg_2, addr + 1);

        return;
    }

    case AM_R_RD:
    {
        assert(ctx.current_instruction->reg_1 != RT_NONE);
        assert(ctx.current_instruction->reg_2 != RT_NONE);
        assert(ctx.current_instruction->cond == CT_NONE);

        u16 addr = cpu_read_reg(ctx.current_instruction->reg_2);
        ctx.fetched_data = bus_read(addr);
        emu_cycles(1);
        cpu_write_reg(ctx.current_instruction->reg_2, addr - 1);

        return;
    }

    case AM_RI_R:
    {
        assert(ctx.current_instruction->reg_1 != RT_NONE);
        assert(ctx.current_instruction->reg_2 != RT_NONE);
        assert(ctx.current_instruction->cond == CT_NONE);

        u16 addr = cpu_read_reg(ctx.current_instruction->reg_1);
        ctx.fetched_data = cpu_read_reg(ctx.current_instruction->reg_2);
        ctx.mem_dest = addr;
        ctx.dest_is_mem = true;
        cpu_write_reg(ctx.current_instruction->reg_1, addr + 1);

        return;
    }

    case AM_RD_R:
    {
        assert(ctx.current_instruction->reg_1 != RT_NONE);
        assert(ctx.current_instruction->reg_2 != RT_NONE);
        assert(ctx.current_instruction->cond == CT_NONE);

        u16 addr = cpu_read_reg(ctx.current_instruction->reg_1);
        ctx.fetched_data = cpu_read_reg(ctx.current_instruction->reg_2);
        ctx.mem_dest = addr;
        ctx.dest_is_mem = true;
        cpu_write_reg(ctx.current_instruction->reg_1, addr - 1);

        return;
    }

    case AM_A8_R:
    {
        assert(ctx.current_instruction->reg_1 == RT_NONE);
        assert(ctx.current_instruction->reg_2 != RT_NONE);
        assert(ctx.current_instruction->cond == CT_NONE);

        ctx.mem_dest = bus_read(ctx.regs.pc) | 0xFF00;
        emu_cycles(1);
        ctx.regs.pc++;

        ctx.dest_is_mem = true;

        return;
    }

    case AM_D8:
        assert(ctx.current_instruction->reg_1 == RT_NONE);
    case AM_R_A8:
    case AM_R_D8:
        assert(ctx.current_instruction->reg_2 == RT_NONE);
    case AM_HL_SPR:
    {
        assert(ctx.current_instruction->mode == AM_D8 || ctx.current_instruction->cond == CT_NONE);

        ctx.fetched_data = bus_read(ctx.regs.pc);
        emu_cycles(1);
        ctx.regs.pc++;

        return;
    }

    case AM_A16_R:
    {
        assert(ctx.current_instruction->reg_1 == RT_NONE);
        assert(ctx.current_instruction->reg_2 != RT_NONE);
        assert(ctx.current_instruction->cond == CT_NONE);

        u16 lo = bus_read(ctx.regs.pc);
        emu_cycles(1);
        ctx.regs.pc++;

        u16 hi = bus_read(ctx.regs.pc);
        emu_cycles(1);
        ctx.regs.pc++;

        ctx.mem_dest = (hi << 8) | lo;
        ctx.dest_is_mem = true;

        ctx.fetched_data = cpu_read_reg(ctx.current_instruction->reg_2);

        return;
    }

    case AM_MR_D8:
    {
        assert(ctx.current_instruction->reg_1 != RT_NONE);
        assert(ctx.current_instruction->reg_2 == RT_NONE);
        assert(ctx.current_instruction->cond == CT_NONE);

        ctx.fetched_data = bus_read(ctx.regs.pc);
        emu_cycles(1);
        ctx.regs.pc++;

        ctx.mem_dest = cpu_read_reg(ctx.current_instruction->reg_1);
        ctx.dest_is_mem = true;

        if (ctx.current_instruction->reg_1 == RT_C)
            ctx.mem_dest |= 0xFF00;

        return;
    }

    default:
        printf("Unhandled address mode %d\n", ctx.current_instruction->mode);
        abort();
        return;
    }
}

static char *rt_lookup[] = {
    "NONE",
    "A",
    "F",
    "B",
    "C",
    "D",
    "E",
    "H",
    "L",
    "AF",
    "BC",
    "DE",
    "HL",
    "SP",
    "PC",
};

const char *instr_to_str(cpu_context *ctx)
{
    static char str[16];
    instruction *inst = ctx->current_instruction;
    snprintf(str, sizeof(str), "%s", instruction_name(inst));

    switch (inst->mode)
    {
    case AM_NONE:
        return str;

    case AM_R_D16:
    case AM_R_A16:
        snprintf(str, sizeof(str), "%s %s,$%04X", instruction_name(inst), rt_lookup[inst->reg_1], ctx->fetched_data);
        return str;

    case AM_R:
        snprintf(str, sizeof(str), "%s %s", instruction_name(inst), rt_lookup[inst->reg_1]);
        return str;

    case AM_R_R:
        snprintf(str, sizeof(str), "%s %s,%s", instruction_name(inst), rt_lookup[inst->reg_1], rt_lookup[inst->reg_2]);
        return str;

    case AM_MR_R:
        snprintf(str, sizeof(str), "%s (%s),%s", instruction_name(inst), rt_lookup[inst->reg_1], rt_lookup[inst->reg_2]);
        return str;

    case AM_MR:
        snprintf(str, sizeof(str), "%s (%s)", instruction_name(inst), rt_lookup[inst->reg_1]);
        return str;

    case AM_R_MR:
        snprintf(str, sizeof(str), "%s %s,(%s)", instruction_name(inst), rt_lookup[inst->reg_1], rt_lookup[inst->reg_2]);
        return str;

    case AM_R_D8:
        snprintf(str, sizeof(str), "%s %s,$%02X", instruction_name(inst), rt_lookup[inst->reg_1], ctx->fetched_data);
        return str;

    case AM_R_A8:
        snprintf(str, sizeof(str), "%s %s,$%02X", instruction_name(inst), rt_lookup[inst->reg_1], ctx->fetched_data);
        return str;

    case AM_R_RI:
        snprintf(str, sizeof(str), "%s %s,(%s+)", instruction_name(inst), rt_lookup[inst->reg_1], rt_lookup[inst->reg_2]);
        return str;

    case AM_R_RD:
        snprintf(str, sizeof(str), "%s %s,(%s-)", instruction_name(inst), rt_lookup[inst->reg_1], rt_lookup[inst->reg_2]);
        return str;

    case AM_RI_R:
        snprintf(str, sizeof(str), "%s (%s+),%s", instruction_name(inst), rt_lookup[inst->reg_1], rt_lookup[inst->reg_2]);
        return str;

    case AM_RD_R:
        snprintf(str, sizeof(str), "%s (%s-),%s", instruction_name(inst), rt_lookup[inst->reg_1], rt_lookup[inst->reg_2]);
        return str;

    case AM_A8_R:
        snprintf(str, sizeof(str), "%s $%02X,%s", instruction_name(inst), bus_read(ctx->regs.pc - 1), rt_lookup[inst->reg_2]);
        return str;

    case AM_HL_SPR:
        snprintf(str, sizeof(str), "%s (%s),SP+%d", instruction_name(inst), rt_lookup[inst->reg_1], ctx->fetched_data & 0xFF);
        return str;

    case AM_D8:
        snprintf(str, sizeof(str), "%s $%02X", instruction_name(inst), ctx->fetched_data);
        return str;

    case AM_D16:
        snprintf(str, sizeof(str), "%s $%04X", instruction_name(inst), ctx->fetched_data);
        return str;

    case AM_MR_D8:
        snprintf(str, sizeof(str), "%s (%s),$%02X", instruction_name(inst), rt_lookup[inst->reg_1], ctx->fetched_data & 0xFF);
        return str;

    case AM_A16_R:
        snprintf(str, sizeof(str), "%s ($%04X),%s", instruction_name(inst), ctx->fetched_data, rt_lookup[inst->reg_2]);
        return str;

    default:
        fprintf(stderr, "INVALID ADDRESS MODE: %d\n", inst->mode);
        NO_IMPL(NULL);
    }
}
