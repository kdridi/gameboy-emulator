#include <ppu.h>
#include <lcd.h>
#include <ppu_sm.h>

static ppu_context ctx = {0};

#undef PPU
#define PPU (ctx)

#undef PFC
#define PFC (PPU.pfc)

#undef PIXEL_FIFO
#define PIXEL_FIFO (PFC.pixel_fifo)

#undef VIDEO_BUFFER
#define VIDEO_BUFFER (PPU.video_buffer)

ppu_context *ppu_get_context(void)
{
    return &ctx;
}

void ppu_init(void)
{
    ctx.current_frame = 0;
    ctx.line_ticks = 0;
    ctx.video_buffer = malloc(YRES * XRES * sizeof(32));

    ctx.pfc.line_x = 0;
    ctx.pfc.pushed_x = 0;
    ctx.pfc.fetch_x = 0;
    ctx.pfc.pixel_fifo.size = 0;
    ctx.pfc.pixel_fifo.head = ctx.pfc.pixel_fifo.tail = NULL;
    ctx.pfc.cur_fetch_state = FS_TILE;

    ctx.line_sprites = 0;
    ctx.fetched_entry_count = 0;
    ctx.window_line = 0;

    lcd_init();
    LCDS_MODE_SET(MODE_OAM);

    memset(ctx.oam_ram, 0, sizeof(ctx.oam_ram));
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
    }
}

void ppu_oam_write(u16 address, u8 value)
{
    if (address >= ADDR_OAM_START)
        address -= ADDR_OAM_START;

    u8 *p = (u8 *)ctx.oam_ram;
    p[address] = value;
}

u8 ppu_oam_read(u16 address)
{
    if (address >= ADDR_OAM_START)
        address -= ADDR_OAM_START;

    u8 *p = (u8 *)ctx.oam_ram;
    return p[address];
}

void ppu_vram_write(u16 address, u8 value)
{
    ctx.vram[address - ADDR_VRAM_START] = value;
}

u8 ppu_vram_read(u16 address)
{
    return ctx.vram[address - ADDR_VRAM_START];
}
