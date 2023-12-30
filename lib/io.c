#include <io.h>
#include <timer.h>
#include <cpu.h>
#include <dma.h>
#include <lcd.h>
#include <gamepad.h>

static char serial_data[2];

u8 io_read(u16 address)
{
    if (address == JOYPAD)
        return gamepad_read();

    if (address == SERIAL_TRANSFER_DATA)
        return serial_data[0];

    if (address == SERIAL_TRANSFER_CONTROL)
        return serial_data[1];

    if (BETWEEN(address, TIMER_DIVIDER, TIMER_CONTROL))
        return timer_read(address);

    if (BETWEEN(address, ADDR_LCD_START, ADDR_LCD_END))
        return lcd_read(address);

    if (address == INTERRUPT_FLAG)
        return cpu_get_int_flags();

    // ignore sound
    if (BETWEEN(address, 0xFF10, 0xFF3F))
        return 0;

    printf("UNSUPPORTED bus_read(%04X)\n", address);
    return 0;
}

void io_write(u16 address, u8 value)
{
    if (address == JOYPAD)
    {
        gamepad_write(value);
        return;
    }

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

    if (BETWEEN(address, ADDR_LCD_START, ADDR_LCD_END))
    {
        lcd_write(address, value);
        return;
    }

    if (address == INTERRUPT_FLAG)
    {
        cpu_set_int_flags(value);
        return;
    }

    // ignore sound
    if (BETWEEN(address, 0xFF10, 0xFF3F))
    {
        return;
    }

    printf("UNSUPPORTED bus_write(%04X)\n", address);
}
