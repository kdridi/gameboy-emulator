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

    ctx.halted = false;
    ctx.stepping = false;
    ctx.interrupts_enabled = true;
}

static void fetch_instruction(void)
{
    ctx.current_opcode = bus_read(ctx.regs.pc++);
    ctx.current_instruction = instruction_by_opcode(ctx.current_opcode);
}

static void fetch_data(void)
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
    IN_PROC proc = inst_get_processor(ctx.current_instruction->type);
    if (proc == NULL)
        NO_IMPL();
    proc(&ctx);
}

bool cpu_step(void)
{
    if (!ctx.halted)
    {
        u16 pc = ctx.regs.pc;

        fetch_instruction();
        fetch_data();

        printf("%04X: %-7s (%02X %02X %02X %02X) A: %02X F: %02X B: %02X C: %02X D: %02X E: %02X H: %02X L: %02X\n", pc, instruction_name(ctx.current_instruction), ctx.current_opcode, bus_read(pc + 1), bus_read(pc + 2), bus_read(pc + 3), ctx.regs.a, ctx.regs.f, ctx.regs.b, ctx.regs.c, ctx.regs.d, ctx.regs.e, ctx.regs.h, ctx.regs.l);

        if (ctx.current_instruction == NULL)
        {
            printf("Unknown instruction: 0x%02X\n", ctx.current_opcode);
            exit(84);
        }

        execute();
    }

    return true;
}

void cpu_set_flags(u8 z, u8 n, u8 h, u8 c)
{
    if (z != 0xff)
        BIT_SET(ctx.regs.f, 7, z);
    if (n != 0xff)
        BIT_SET(ctx.regs.f, 6, n);
    if (h != 0xff)
        BIT_SET(ctx.regs.f, 5, h);
    if (c != 0xff)
        BIT_SET(ctx.regs.f, 4, c);
}
