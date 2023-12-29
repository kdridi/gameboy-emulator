#include <ppu.h>

static ppu_context ctx = {0};

void ppu_init(void)
{
}

void ppu_tick(void)
{
}

static u8 *ppu_addr(char *type, u16 address, u16 start, u16 end, u8 *base, size_t block_size)
{
    if (BETWEEN(address, start, end))
        address -= start;
    return base + address;
}

static u8 *ppu_oam_addr(u16 address)
{
    return ppu_addr("ppu_oam", address, 0xFE00, 0xFEA0, (u8 *)ctx.oam_ram, sizeof(*ctx.oam_ram));
}

void ppu_oam_write(u16 address, u8 value)
{
    u8 *addr = ppu_oam_addr(address);
    *addr = value;
}

u8 ppu_oam_read(u16 address)
{
    u8 *addr = ppu_oam_addr(address);
    return *addr;
}

static u8 *ppu_vram_addr(u16 address)
{
    return ppu_addr("ppu_vram", address, ADDR_VRAM_START, ADDR_VRAM_END, (u8 *)ctx.vram, sizeof(*ctx.vram));
}

void ppu_vram_write(u16 address, u8 value)
{
    u8 *addr = ppu_vram_addr(address);
    *addr = value;
}

u8 ppu_vram_read(u16 address)
{
    u8 *addr = ppu_vram_addr(address);
    return *addr;
}
