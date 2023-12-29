#include <ppu.h>
#include <lcd.h>
#include <ppu_sm.h>

static ppu_context ctx = {0};

ppu_context *ppu_get_context(void)
{
    return &ctx;
}

void ppu_init(void)
{
    ctx.current_frame = 0;
    ctx.line_ticks = 0;
    ctx.video_buffer = calloc(XRES * YRES, sizeof(u32));

    lcd_init();
    LCDS_MODE_SET(MODE_OAM);

    memset(ctx.oam_ram, 0, sizeof(ctx.oam_ram));
    memset(ctx.vram, 0, sizeof(ctx.vram));
    memset(ctx.video_buffer, 0, YRES * XRES * sizeof(u32));
}

void ppu_tick(void)
{
    ctx.line_ticks++;

    switch (LCDS_MODE)
    {
    case MODE_OAM:
        ppu_mode_oam();
        break;
    case MODE_XFER:
        ppu_mode_xfer();
        break;
    case MODE_VBLANK:
        ppu_mode_vblank();
        break;
    case MODE_HBLANK:
        ppu_mode_hblank();
        break;
    default:
        NO_IMPL();
    }
}

static u8 *ppu_addr(char *type, u16 address, u16 start, u16 end, u8 *base, size_t block_size)
{
    if (BETWEEN(address, start, end))
        return base + (address - start);

    NO_IMPL(NULL);
}

static u8 *ppu_oam_addr(u16 address)
{
    return ppu_addr("ppu_oam", address, ADDR_OAM_START, ADDR_OAM_END, (u8 *)ctx.oam_ram, sizeof(*ctx.oam_ram));
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
