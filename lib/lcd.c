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

static void update_palette(u8 palette_data, u8 pal)
{
    u32 *p_colors = ctx.bg_colors;

    switch (pal)
    {
    case 1:
        p_colors = ctx.sp1_colors;
        break;
    case 2:
        p_colors = ctx.sp2_colors;
        break;
    }

    p_colors[0] = colors_default[(palette_data >> 0) & 0b11];
    p_colors[1] = colors_default[(palette_data >> 2) & 0b11];
    p_colors[2] = colors_default[(palette_data >> 4) & 0b11];
    p_colors[3] = colors_default[(palette_data >> 6) & 0b11];
}

void lcd_write(u16 addr, u8 value)
{
    u8 offset = (addr - ADDR_LCD_START);
    u8 *p = (u8 *)&ctx;
    p[offset] = value;

    if (offset == 6)
        dma_start(value);

    if (addr == 0xFF47)
        update_palette(value, 0);
    else if (addr == 0xFF48)
        update_palette(value & 0b11111100, 1);
    else if (addr == 0xFF49)
        update_palette(value & 0b11111100, 1);
}

u8 lcd_read(u16 addr)
{
    u8 offset = (addr - ADDR_LCD_START);
    u8 *p = (u8 *)&ctx;
    return p[offset];
}
