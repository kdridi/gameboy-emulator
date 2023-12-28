#include <io.h>
#include <timer.h>
#include <cpu.h>

static char serial_data[2];

u8 io_read(u16 address)
{
    if (address == SERIAL_TRANSFER_DATA)
        return serial_data[0];

    if (address == SERIAL_TRANSFER_CONTROL)
        return serial_data[1];

    if (BETWEEN(address, TIMER_DIVIDER, TIMER_CONTROL))
        return timer_read(address);

    if (address == INTERRUPT_FLAG)
        return cpu_get_int_flags();

    printf("UNSUPPORTED bus_read(%04X)\n", address);
    return 0;
}

void io_write(u16 address, u8 value)
{
    if (address == SERIAL_TRANSFER_DATA)
    {
        serial_data[0] = value;
        return;
    }

    if (address == SERIAL_TRANSFER_CONTROL)
    {
        serial_data[1] = value;
        return;
    }

    if (BETWEEN(address, TIMER_DIVIDER, TIMER_CONTROL))
    {
        timer_write(address, value);
        return;
    }

    if (address == INTERRUPT_FLAG)
    {
        cpu_set_int_flags(value);
        return;
    }

    printf("UNSUPPORTED bus_write(%04X)\n", address);
}