#include <cpu.h>
#include <bus.h>
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

void cpu_write_reg(reg_type rt, u16 value)
{
    // clang-format off
    switch (rt)
    {
    case RT_A: ctx.regs.a = value; break;
    case RT_F: ctx.regs.f = value; break;
    case RT_B: ctx.regs.b = value; break;
    case RT_C: ctx.regs.c = value; break;
    case RT_D: ctx.regs.d = value; break;
    case RT_E: ctx.regs.e = value; break;
    case RT_H: ctx.regs.h = value; break;
    case RT_L: ctx.regs.l = value; break;
    case RT_AF: ctx.regs.a = value >> 8; ctx.regs.f = value & 0xFF; break;
    case RT_BC: ctx.regs.b = value >> 8; ctx.regs.c = value & 0xFF; break;
    case RT_DE: ctx.regs.d = value >> 8; ctx.regs.e = value & 0xFF; break;
    case RT_HL: ctx.regs.h = value >> 8; ctx.regs.l = value & 0xFF; break;
    case RT_SP: ctx.regs.sp = value; break;
    case RT_PC: ctx.regs.pc = value; break;
    default:
        printf("Unhandled register type %d\n", rt);
        abort();
    }
    // clang-format on
}

u8 cpu_read_reg8(reg_type rt)
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
    case RT_HL: return bus_read(cpu_read_reg(RT_HL));
    default:
        printf("**ERR INVALID REG8: %d\n", rt);
        NO_IMPL(0);
    }
    // clang-format on
}

void cpu_write_reg8(reg_type rt, u8 value)
{
    // clang-format off
    switch (rt)
    {
    case RT_A: ctx.regs.a = value & 0xFF; break;
    case RT_F: ctx.regs.f = value & 0xFF; break;
    case RT_B: ctx.regs.b = value & 0xFF; break;
    case RT_C: ctx.regs.c = value & 0xFF; break;
    case RT_D: ctx.regs.d = value & 0xFF; break;
    case RT_E: ctx.regs.e = value & 0xFF; break;
    case RT_H: ctx.regs.h = value & 0xFF; break;
    case RT_L: ctx.regs.l = value & 0xFF; break;
    case RT_HL: bus_write(cpu_read_reg(RT_HL), value & 0xFF); break;
    default:
        printf("**ERR INVALID REG8: %d\n", rt);
        abort();
    }
    // clang-format on
}

void cpu_init(void)
{
    ctx.regs.a = 0x00;
    ctx.regs.f = 0x00;
    ctx.regs.b = 0x00;
    ctx.regs.c = 0x13;
    ctx.regs.d = 0x00;
    ctx.regs.e = 0xD8;
    ctx.regs.h = 0x01;
    ctx.regs.l = 0x4D;
    ctx.regs.sp = 0xFFFE;
    ctx.regs.pc = 0x0100;
    ctx.regs.spc = 0;

    ctx.halted = false;
    ctx.stepping = false;
    ctx.interrupts_enabled = true;
}

static void fetch_instruction(void)
{
    ctx.current_opcode = bus_read(ctx.regs.pc++);
    ctx.current_instruction = instruction_by_opcode(ctx.current_opcode);
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
        cpu_fetch_data();

        printf("%08llX - ", emu_get_context()->ticks);
        printf("%04X: %-7s (%02X %02X %02X %02X) A: %02X F: %c%c%c%c BC: %02X%02X DE: %02X%02X HL: %02X%02X SP: %04X", pc, instruction_name(ctx.current_instruction), ctx.current_opcode, bus_read(pc + 1), bus_read(pc + 2), bus_read(pc + 3), ctx.regs.a, ctx.regs.f & 0x80 ? 'Z' : '-', ctx.regs.f & 0x40 ? 'N' : '-', ctx.regs.f & 0x20 ? 'H' : '-', ctx.regs.f & 0x10 ? 'C' : '-', ctx.regs.b, ctx.regs.c, ctx.regs.d, ctx.regs.e, ctx.regs.h, ctx.regs.l, ctx.regs.sp);
        if (ctx.regs.spc > 0)
            printf(" STACK:");
        for (u8 i = 0; i < ctx.regs.spc; i++)
            printf(" %02hhX", bus_read(ctx.regs.sp + i) & 0xFF);
        printf("\n");

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

void cpu_set_interrupt_flags(u8 flags)
{
    ctx.interrupt_flags = flags;
}

u8 cpu_get_interrupt_flags()
{
    return ctx.interrupt_flags;
}

cpu_registers *cpu_get_registers(void)
{
    return &ctx.regs;
}
