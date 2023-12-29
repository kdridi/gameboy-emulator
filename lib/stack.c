#include <stack.h>
#include <cpu.h>
#include <bus.h>

void stack_push(u8 data)
{
    REGS->sp--;
    bus_write(REGS->sp, data);
}

void stack_push16(u16 data)
{
    stack_push((data >> 8) & 0xFF);
    stack_push((data >> 0) & 0xFF);
}

u8 stack_pop()
{
    u8 data = bus_read(REGS->sp);
    REGS->sp++;
    return data;
}

u16 stack_pop16()
{
    u16 lo = stack_pop() << 0;
    u16 hi = stack_pop() << 8;
    return hi | lo;
}
