#include <dbg.h>
#include <bus.h>

static char dbg_message[1024] = {0};
static u64 dbg_size = 0;

void dbg_update()
{
    if (bus_read(SERIAL_TRANSFER_CONTROL) == 0x81)
    {
        char c = bus_read(SERIAL_TRANSFER_DATA);
        dbg_message[dbg_size++] = c;
        dbg_message[dbg_size] = '\0';
        bus_write(SERIAL_TRANSFER_CONTROL, 0x00);
    }
}

void dbg_print()
{
    if (dbg_size > 0)
    {
        printf("DBG: %s\n", dbg_message);
        // dbg_size = 0;
    }
}
