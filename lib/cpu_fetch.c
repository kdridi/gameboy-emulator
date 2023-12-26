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
        return;

    case AM_R:
    case AM_R_R:
        ctx.fetched_data = cpu_read_reg(ctx.current_instruction->reg_1);
        return;

    case AM_R_D8:
        ctx.fetched_data = bus_read(ctx.regs.pc);
        emu_cycles(1);
        ctx.regs.pc++;
        return;

    case AM_D16:
    case AM_R_D16:
    {
        u16 lo = bus_read(ctx.regs.pc);
        emu_cycles(1);
        ctx.regs.pc++;

        u16 hi = bus_read(ctx.regs.pc);
        emu_cycles(1);
        ctx.regs.pc++;

        ctx.fetched_data = (hi << 8) | lo;

        return;
    }

    case AM_MR_R:
    {
        ctx.fetched_data = cpu_read_reg(ctx.current_instruction->reg_2);
        ctx.mem_dest = cpu_read_reg(ctx.current_instruction->reg_1);
        ctx.dest_is_mem = true;

        if (ctx.current_instruction->reg_1 == RT_C)
            ctx.mem_dest |= 0xFF00;

        return;
    }

    case AM_R_MR:
    {
        u16 addr = cpu_read_reg(ctx.current_instruction->reg_2);
        if (ctx.current_instruction->reg_2 == RT_C)
            addr |= 0xFF00;

        ctx.fetched_data = bus_read(addr);
        emu_cycles(1);

        return;
    }

    case AM_R_HLI:
    {
        assert(ctx.current_instruction->reg_2 == RT_HL);

        u16 addr = cpu_read_reg(RT_HL);
        ctx.fetched_data = bus_read(addr);
        emu_cycles(1);
        cpu_write_reg(RT_HL, addr + 1);

        return;
    }

    case AM_R_HLD:
    {
        assert(ctx.current_instruction->reg_2 == RT_HL);

        u16 addr = cpu_read_reg(RT_HL);
        ctx.fetched_data = bus_read(addr);
        emu_cycles(1);
        cpu_write_reg(RT_HL, addr - 1);

        return;
    }

    case AM_HLI_R:
    {
        assert(ctx.current_instruction->reg_1 == RT_HL);

        u16 addr = cpu_read_reg(RT_HL);
        ctx.fetched_data = cpu_read_reg(ctx.current_instruction->reg_2);
        ctx.mem_dest = addr;
        ctx.dest_is_mem = true;
        cpu_write_reg(RT_HL, addr + 1);

        return;
    }

    case AM_HLD_R:
    {
        assert(ctx.current_instruction->reg_1 == RT_HL);

        u16 addr = cpu_read_reg(RT_HL);
        ctx.fetched_data = cpu_read_reg(ctx.current_instruction->reg_2);
        ctx.mem_dest = addr;
        ctx.dest_is_mem = true;
        cpu_write_reg(RT_HL, addr - 1);

        return;
    }

    case AM_A8_R:
    {
        ctx.fetched_data = cpu_read_reg(ctx.current_instruction->reg_1);

        ctx.mem_dest = bus_read(ctx.regs.pc) | 0xFF00;
        emu_cycles(1);
        ctx.regs.pc++;

        ctx.dest_is_mem = true;

        return;
    }

    case AM_D8:
    case AM_R_A8:
    case AM_HL_SPR:
    {
        ctx.fetched_data = bus_read(ctx.regs.pc);
        emu_cycles(1);
        ctx.regs.pc++;

        return;
    }

    case AM_A16_R:
    case AM_D16_R:
    {
        u16 lo = bus_read(ctx.regs.pc);
        emu_cycles(1);
        ctx.regs.pc++;

        u16 hi = bus_read(ctx.regs.pc);
        emu_cycles(1);
        ctx.regs.pc++;

        ctx.mem_dest = (hi << 8) | lo;
        ctx.dest_is_mem = true;

        ctx.fetched_data = cpu_read_reg(ctx.current_instruction->reg_1);

        return;
    }

    case AM_MR_D8:
    {
        ctx.fetched_data = bus_read(ctx.regs.pc);
        emu_cycles(1);
        ctx.regs.pc++;

        ctx.mem_dest = cpu_read_reg(ctx.current_instruction->reg_1);
        ctx.dest_is_mem = true;

        if (ctx.current_instruction->reg_1 == RT_C)
            ctx.mem_dest |= 0xFF00;

        return;
    }

    case AM_MR:
    {
        ctx.mem_dest = cpu_read_reg(ctx.current_instruction->reg_1);
        ctx.dest_is_mem = true;

        if (ctx.current_instruction->reg_1 == RT_C)
            ctx.mem_dest |= 0xFF00;

        ctx.fetched_data = bus_read(cpu_read_reg(ctx.current_instruction->reg_1));
        emu_cycles(1);

        return;
    }

    case AM_R_A16:
    {
        u16 lo = bus_read(ctx.regs.pc);
        emu_cycles(1);
        ctx.regs.pc++;

        u16 hi = bus_read(ctx.regs.pc);
        emu_cycles(1);
        ctx.regs.pc++;

        ctx.fetched_data = bus_read((hi << 8) | lo);
        emu_cycles(1);

        return;
    }

    default:
        printf("Unhandled address mode %d\n", ctx.current_instruction->mode);
        abort();
        return;
    }
}
