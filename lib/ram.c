#include <ram.h>

#define WRAM_SIZE (1 << 13)
#define HRAM_SIZE (1 << 7)

typedef struct
{
    u8 wram[WRAM_SIZE]; // 8KB
    u8 hram[HRAM_SIZE]; // 128B
} ram_context;

static ram_context ctx;

u8 wram_read(u16 address)
{
    return ctx.wram[address & (WRAM_SIZE - 1)];
}

void wram_write(u16 address, u8 value)
{
    ctx.wram[address & (WRAM_SIZE - 1)] = value;
}

u8 hram_read(u16 address)
{
    return ctx.hram[address & (HRAM_SIZE - 1)];
}

void hram_write(u16 address, u8 value)
{
    ctx.hram[address & (HRAM_SIZE - 1)] = value;
}
