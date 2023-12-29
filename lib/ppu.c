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
    PPU->current_frame = 0;
    PPU->line_ticks = 0;

    VIDEO_BUFFER = calloc(XRES * YRES, sizeof(u32));

    PFC->line_x = 0;
    PFC->pushed_x = 0;
    PFC->fetch_x = 0;
    PFC->pixel_fifo.size = 0;
    PFC->pixel_fifo.head = NULL;
    PFC->pixel_fifo.tail = NULL;
    PFC->cur_fetch_state = FS_TILE;

    lcd_init();
    LCDS_MODE_SET(MODE_OAM);

    memset(PPU->oam_ram, 0, sizeof(PPU->oam_ram));
    memset(PPU->vram, 0, sizeof(PPU->vram));
    memset(VIDEO_BUFFER, 0, YRES * XRES * sizeof(u32));
}

void ppu_tick(void)
{
    PPU->line_ticks++;

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
    return ppu_addr("ppu_oam", address, ADDR_OAM_START, ADDR_OAM_END, (u8 *)PPU->oam_ram, sizeof(*PPU->oam_ram));
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
    return ppu_addr("ppu_vram", address, ADDR_VRAM_START, ADDR_VRAM_END, (u8 *)PPU->vram, sizeof(*PPU->vram));
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
