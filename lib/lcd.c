#include <lcd.h>
#include <ppu.h>
#include <dma.h>

static lcd_context ctx = {0};

static u32 colors_default[4] = {COLOR0, COLOR1, COLOR2, COLOR3};

void lcd_init(void)
{
    ctx.lcdc = 0x91;
    ctx.scroll_x = 0;
    ctx.scroll_y = 0;
    ctx.ly = 0;
    ctx.ly_compare = 0;
    ctx.bg_palette = 0xFC;
    ctx.obj_palette[0] = 0xFF;
    ctx.obj_palette[1] = 0xFF;
    ctx.win_y = 0;
    ctx.win_x = 0;

    for (int i = 0; i < 4; i++)
    {
        ctx.bg_colors[i] = colors_default[i];
        ctx.sp1_colors[i] = colors_default[i];
        ctx.sp2_colors[i] = colors_default[i];
    }
}

lcd_context *lcd_get_context(void)
{
    return &ctx;
}

u8 lcd_read(u16 address)
{
    u8 offset = (address - ADDR_LCD_START);
    u8 *p = (u8 *)&ctx;

    return p[offset];
}

static void palette_update(u32 *palette, u8 palette_data)
{
    palette[0] = colors_default[(palette_data >> 0) & 0x3];
    palette[1] = colors_default[(palette_data >> 2) & 0x3];
    palette[2] = colors_default[(palette_data >> 4) & 0x3];
    palette[3] = colors_default[(palette_data >> 6) & 0x3];
}

void lcd_write(u16 address, u8 value)
{
    u8 offset = (address - ADDR_LCD_START);
    u8 *p = (u8 *)&ctx;
    p[offset] = value;

    if (offset == 6)
    {
        dma_start(value);
        return;
    }

    if (address == 0xFF47)
    {
        palette_update(ctx.bg_colors, value);
        return;
    }

    if (address == 0xFF48)
    {
        palette_update(ctx.sp1_colors, value & 0xFC);
        return;
    }

    if (address == 0xFF49)
    {
        palette_update(ctx.sp2_colors, value & 0xFC);
        return;
    }
}
