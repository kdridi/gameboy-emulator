#include <cpu.h>
#include <bus.h>
#include <emu.h>
#include <interrupts.h>
#include <dbg.h>
#include <timer.h>

cpu_context ctx = {0};

#undef REGS

#define CPU (ctx)
#define REGS (CPU.regs)

#define CPU_DEBUG 0

u16 cpu_read_reg(reg_type rt)
{
    // clang-format off
    switch (rt)
    {
    case RT_A: return REGS.a;
    case RT_F: return REGS.f;
    case RT_B: return REGS.b;
    case RT_C: return REGS.c;
    case RT_D: return REGS.d;
    case RT_E: return REGS.e;
    case RT_H: return REGS.h;
    case RT_L: return REGS.l;
    case RT_AF: return (REGS.a << 8) | REGS.f;
    case RT_BC: return (REGS.b << 8) | REGS.c;
    case RT_DE: return (REGS.d << 8) | REGS.e;
    case RT_HL: return (REGS.h << 8) | REGS.l;
    case RT_SP: return REGS.sp;
    case RT_PC: return REGS.pc;
    default:
        printf("Unhandled register type %d\n", rt);
        assert(false);
        return 0;
    }
    // clang-format on
}

void cpu_write_reg(reg_type rt, u16 value)
{
    // clang-format off
    switch (rt)
    {
        case RT_A: REGS.a = value; break;
        case RT_F: REGS.f = value; break;
        case RT_B: REGS.b = value; break;
        case RT_C: REGS.c = value; break;
        case RT_D: REGS.d = value; break;
        case RT_E: REGS.e = value; break;
        case RT_H: REGS.h = value; break;
        case RT_L: REGS.l = value; break;
        case RT_AF: REGS.a = (value >> 8) & 0xFF; REGS.f =(value >> 0) & 0xFF; break;
        case RT_BC: REGS.b = (value >> 8) & 0xFF; REGS.c =(value >> 0) & 0xFF; break;
        case RT_DE: REGS.d = (value >> 8) & 0xFF; REGS.e =(value >> 0) & 0xFF; break;
        case RT_HL: REGS.h = (value >> 8) & 0xFF; REGS.l =(value >> 0) & 0xFF; break;
        case RT_SP: REGS.sp = value; break;
        case RT_PC: REGS.pc = value; break;
        default: assert(false); NO_IMPL();
    }
    // clang-format on
}

u8 cpu_read_reg8(reg_type rt)
{
    // clang-format off
    switch (rt)
    {
        case RT_A: return REGS.a;
        case RT_F: return REGS.f;
        case RT_B: return REGS.b;
        case RT_C: return REGS.c;
        case RT_D: return REGS.d;
        case RT_E: return REGS.e;
        case RT_H: return REGS.h;
        case RT_L: return REGS.l;
        case RT_HL: return bus_read(cpu_read_reg(RT_HL));
        default: assert(false); NO_IMPL(0);
    }
    // clang-format on
}

void cpu_write_reg8(reg_type rt, u8 value)
{
    // clang-format off
    switch (rt)
    {
        case RT_A: REGS.a = value; break;
        case RT_F: REGS.f = value; break;
        case RT_B: REGS.b = value; break;
        case RT_C: REGS.c = value; break;
        case RT_D: REGS.d = value; break;
        case RT_E: REGS.e = value; break;
        case RT_H: REGS.h = value; break;
        case RT_L: REGS.l = value; break;
        case RT_HL: bus_write(cpu_read_reg(RT_HL), value); break;
        default: assert(false); NO_IMPL();
    }
    // clang-format on
}

void cpu_init(void)
{
    REGS.a = 0x01;
    REGS.f = 0xB0;
    REGS.b = 0x00;
    REGS.c = 0x13;
    REGS.d = 0x00;
    REGS.e = 0xD8;
    REGS.h = 0x01;
    REGS.l = 0x4D;
    REGS.sp = 0xFFFE;
    REGS.pc = 0x0100;

    CPU.ie_register = 0x00;
    CPU.int_flags = 0x00;
    CPU.int_master_enabled = false;
    CPU.enabling_ime = false;

    TIMER->div = 0xABCC;

    CPU.halted = false;
    CPU.stepping = false;
}

static void fetch_instruction(void)
{
    CPU.current_opcode = bus_read(REGS.pc++);
    CPU.current_instruction = instruction_by_opcode(CPU.current_opcode);
}

static void execute(void)
{
    IN_PROC proc = inst_get_processor(CPU.current_instruction->type);
    if (proc == NULL)
        NO_IMPL();
    proc(&ctx);
}

bool cpu_step(void)
{
    if (!CPU.halted)
    {
#if CPU_DEBUG == 1
        u16 pc = REGS.pc;
#endif

        fetch_instruction();
        emu_cycles(1);
        cpu_fetch_data();

#if CPU_DEBUG == 1
        char flags[16];
        sprintf(flags, "%c%c%c%c",
                REGS.f & (1 << 7) ? 'Z' : '-',
                REGS.f & (1 << 6) ? 'N' : '-',
                REGS.f & (1 << 5) ? 'H' : '-',
                REGS.f & (1 << 4) ? 'C' : '-');

        char inst[16];
        instr_to_str(&ctx, inst);

        printf("%08llX - %04X: %-12s (%02X %02X %02X) A: %02X F: %s BC: %02X%02X DE: %02X%02X HL: %02X%02X\n",
               EMU->ticks,
               pc, inst, CPU.current_opcode,
               bus_read(pc + 1), bus_read(pc + 2), REGS.a, flags, REGS.b, REGS.c,
               REGS.d, REGS.e, REGS.h, REGS.l);
#endif

        if (CPU.current_instruction == NULL)
        {
            printf("Unknown Instruction! %02X\n", CPU.current_opcode);
            exit(-7);
        }

        dbg_update();
        dbg_print();

        execute();
    }
    else
    {
        emu_cycles(1);

        if (CPU.int_flags != 0)
        {
            CPU.halted = false;
        }
    }

    if (CPU.int_master_enabled)
    {
        cpu_handle_interrupts(&ctx);
        CPU.enabling_ime = false;
    }

    if (CPU.enabling_ime)
    {
        CPU.int_master_enabled = true;
        CPU.enabling_ime = false;
    }

    return true;
}

void cpu_set_flags(u8 z, u8 n, u8 h, u8 c)
{
    assert(z == 0xff || z == 0 || z == 1);
    if (z != 0xff)
        BIT_SET(REGS.f, 7, z);

    assert(n == 0xff || n == 0 || n == 1);
    if (n != 0xff)
        BIT_SET(REGS.f, 6, n);

    assert(h == 0xff || h == 0 || h == 1);
    if (h != 0xff)
        BIT_SET(REGS.f, 5, h);

    assert(c == 0xff || c == 0 || c == 1);
    if (c != 0xff)
        BIT_SET(REGS.f, 4, c);
}

u8 cpu_get_ie_register()
{
    return CPU.ie_register;
}

void cpu_set_ie_register(u8 n)
{
    CPU.ie_register = n;
}

cpu_registers *cpu_get_registers(void)
{
    return &CPU.regs;
}

u8 cpu_get_int_flags()
{
    return CPU.int_flags;
}

void cpu_set_int_flags(u8 value)
{
    CPU.int_flags = value;
}

void cpu_request_interrupt(interrupt_type type)
{
    CPU.int_flags |= type;
}
