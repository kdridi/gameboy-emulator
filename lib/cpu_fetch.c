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

void instr_to_str(cpu_context *ctx, char *str)
{
    instruction *inst = ctx->current_instruction;
    sprintf(str, "%s", instruction_name(inst));

    switch (inst->mode)
    {
    case AM_NONE:
        return;

    case AM_R_D16:
    case AM_R_A16:
        sprintf(str, "%s %s,$%04X", instruction_name(inst), rt_lookup[inst->reg_1], ctx->fetched_data);
        return;
    case AM_R:
        sprintf(str, "%s %s", instruction_name(inst), rt_lookup[inst->reg_1]);
        return;

    case AM_R_R:
        sprintf(str, "%s %s,%s", instruction_name(inst), rt_lookup[inst->reg_1], rt_lookup[inst->reg_2]);
        return;

    case AM_MR_R:
        sprintf(str, "%s (%s),%s", instruction_name(inst), rt_lookup[inst->reg_1], rt_lookup[inst->reg_2]);
        return;

    case AM_MR:
        sprintf(str, "%s (%s)", instruction_name(inst), rt_lookup[inst->reg_1]);
        return;

    case AM_R_MR:
        sprintf(str, "%s %s,(%s)", instruction_name(inst), rt_lookup[inst->reg_1], rt_lookup[inst->reg_2]);
        return;

    case AM_R_D8:
        sprintf(str, "%s %s,$%02X", instruction_name(inst), rt_lookup[inst->reg_1], ctx->fetched_data);
        return;

    case AM_R_A8:
        sprintf(str, "%s %s,$%02X", instruction_name(inst), rt_lookup[inst->reg_1], ctx->fetched_data);
        return;

    case AM_R_RI:
        sprintf(str, "%s %s,(%s+)", instruction_name(inst), rt_lookup[inst->reg_1], rt_lookup[inst->reg_2]);
        return;

    case AM_R_RD:
        sprintf(str, "%s %s,(%s-)", instruction_name(inst), rt_lookup[inst->reg_1], rt_lookup[inst->reg_2]);
        return;

    case AM_RI_R:
        sprintf(str, "%s (%s+),%s", instruction_name(inst), rt_lookup[inst->reg_1], rt_lookup[inst->reg_2]);
        return;

    case AM_RD_R:
        sprintf(str, "%s (%s-),%s", instruction_name(inst), rt_lookup[inst->reg_1], rt_lookup[inst->reg_2]);
        return;

    case AM_A8_R:
        sprintf(str, "%s $%02X,%s", instruction_name(inst), bus_read(ctx->regs.pc - 1), rt_lookup[inst->reg_2]);
        return;

    case AM_HL_SPR:
        sprintf(str, "%s (%s),SP+%d", instruction_name(inst), rt_lookup[inst->reg_1], ctx->fetched_data & 0xFF);
        return;

    case AM_D8:
        sprintf(str, "%s $%02X", instruction_name(inst), ctx->fetched_data);
        return;

    case AM_D16:
        sprintf(str, "%s $%04X", instruction_name(inst), ctx->fetched_data);
        return;

    case AM_MR_D8:
        sprintf(str, "%s (%s),$%02X", instruction_name(inst), rt_lookup[inst->reg_1], ctx->fetched_data & 0xFF);
        return;

    case AM_A16_R:
        sprintf(str, "%s ($%04X),%s", instruction_name(inst), ctx->fetched_data, rt_lookup[inst->reg_2]);
        return;

    default:
        fprintf(stderr, "INVALID ADDRESS MODE: %d\n", inst->mode);
        NO_IMPL();
    }
}
