#include <timer.h>
#include <interrupts.h>

static timer_context ctx = {0};

timer_context *timer_get_context(void)
{
    return &ctx;
}

void timer_init(void)
{
    ctx.div = 0xAC00;
    ctx.tima = 0x00;
    ctx.tma = 0x00;
    ctx.tac = 0x00;
}

void timer_tick(void)
{
    u16 prev_div = ctx.div;
    ctx.div++;

    bool timer_update = false;

    switch (ctx.tac & 0x3)
    {
    case 0x0:
        timer_update = BIT(prev_div, 9) && !BIT(ctx.div, 9);
        break;
    case 0x1:
        timer_update = BIT(prev_div, 3) && !BIT(ctx.div, 3);
        break;
    case 0x2:
        timer_update = BIT(prev_div, 5) && !BIT(ctx.div, 5);
        break;
    case 0x3:
        timer_update = BIT(prev_div, 7) && !BIT(ctx.div, 7);
        break;
    }

    if (timer_update && BIT(ctx.tac, 2))
    {
        ctx.tima++;

        if (ctx.tima == 0xFF)
        {
            ctx.tima = ctx.tma;
            cpu_request_interrupt(IT_TIMER);
        }
    }
}

void timer_write(u16 addr, u8 value)
{
    switch (addr)
    {
    case TIMER_DIVIDER:
        ctx.div = 0;
        return;
    case TIMER_COUNTER:
        ctx.tima = value;
        return;
    case TIMER_MODULO:
        ctx.tma = value;
        return;
    case TIMER_CONTROL:
        ctx.tac = value;
        return;
    }

    printf("timer_write: addr=%04X value=%02X\n", addr, value);
    assert(false);
}

u8 timer_read(u16 addr)
{
    switch (addr)
    {
    case TIMER_DIVIDER:
        return ctx.div >> 8;
    case TIMER_COUNTER:
        return ctx.tima;
    case TIMER_MODULO:
        return ctx.tma;
    case TIMER_CONTROL:
        return ctx.tac;
    }

    assert(false);
    return 0;
}
