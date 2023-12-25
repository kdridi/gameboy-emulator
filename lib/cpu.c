#include <cpu.h>
#include <bus.h>
#include <instructions.h>
#include <emu.h>

cpu_context ctx = {0};

u16 cpu_read_reg(reg_type rt)
{
    // clang-format off
    switch (rt)
    {
    case RT_A: return ctx.regs.a;
    case RT_F: return ctx.regs.f;
    case RT_B: return ctx.regs.b;
    case RT_C: return ctx.regs.c;
    case RT_D: return ctx.regs.d;
    case RT_E: return ctx.regs.e;
    case RT_H: return ctx.regs.h;
    case RT_L: return ctx.regs.l;
    case RT_AF: return (ctx.regs.a << 8) | ctx.regs.f;
    case RT_BC: return (ctx.regs.b << 8) | ctx.regs.c;
    case RT_DE: return (ctx.regs.d << 8) | ctx.regs.e;
    case RT_HL: return (ctx.regs.h << 8) | ctx.regs.l;
    case RT_SP: return ctx.regs.sp;
    case RT_PC: return ctx.regs.pc;
    default:
        printf("Unhandled register type %d\n", rt);
        abort();
        return 0;
    }
    // clang-format on
}

void cpu_init(void)
{
    ctx.regs.a = 0x01;
    ctx.regs.f = 0xB0;
    ctx.regs.b = 0x00;
    ctx.regs.c = 0x13;
    ctx.regs.d = 0x00;
    ctx.regs.e = 0xD8;
    ctx.regs.h = 0x01;
    ctx.regs.l = 0x4D;
    ctx.regs.sp = 0xFFFE;
    ctx.regs.pc = 0x0100;
}

static void fetch_instruction(void)
{
    ctx.current_opcode = bus_read(ctx.regs.pc++);
    ctx.current_instruction = instruction_by_opcode(ctx.current_opcode);

    if (ctx.current_instruction == NULL)
    {
        printf("Unknown instruction: 0x%02X\n", ctx.current_opcode);
        abort();
    }
}

static void fetch_data(void)
{
    ctx.mem_dest = 0;
    ctx.dest_is_mem = false;

    switch (ctx.current_instruction->mode)
    {
    case AM_IMP:
        return;

    case AM_R:
        ctx.fetched_data = cpu_read_reg(ctx.current_instruction->reg_1);
        return;

    case AM_R_D8:
        ctx.fetched_data = bus_read(ctx.regs.pc);
        emu_cycles(1);
        ctx.regs.pc++;
        return;
    case AM_D16:
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
    default:
        printf("Unhandled address mode %d\n", ctx.current_instruction->mode);
        abort();
        return;
    }
}

static void execute(void)
{
    printf("\tNot executing yet...\n");
}

bool cpu_step(void)
{
    if (!ctx.halted)
    {
        u16 pc = ctx.regs.pc;

        fetch_instruction();
        fetch_data();

        printf("Executing Instruction: %02X     PC: %04X\n", ctx.current_opcode, pc);

        execute();
    }

    return true;
}
