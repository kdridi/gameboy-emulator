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
    ctx.win_x = 0;
    ctx.win_y = 0;

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
    u8 *palette = NULL;
    u32 *p_colors = NULL;

    switch (pal)
    {
    case 0:
        palette = &ctx.bg_palette;
        p_colors = (u32 *)ctx.bg_colors;
        break;
    case 1:
        palette = &ctx.obj_palette[0];
        p_colors = (u32 *)ctx.sp1_colors;
        break;
    case 2:
        palette = &ctx.obj_palette[1];
        p_colors = (u32 *)ctx.sp2_colors;
        break;
    }

    *palette = palette_data;

    for (int i = 0; i < 4; i++)
        p_colors[i] = colors_default[palette_data >> (i * 2) & 0b11];
}

void lcd_write(u16 addr, u8 value)
{
    u8 offset = addr - ADDR_LCD_START;
    u8 *ptr = (u8 *)lcd_get_context();
    ptr[offset] = value;

    if (offset == 6)
    {
        // 0xFF46 = DMA_TRANSFER
        dma_start(value);
    }

    if (addr == 0xFF47) // BG_PALETTE
        update_palette(value, 0);
    else if (addr == 0xFF48) // OBJ_PALETTE_0
        update_palette(value & 0b11111100, 1);
    else if (addr == 0xFF49) // OBJ_PALETTE_1
        update_palette(value & 0b11111100, 2);
}

u8 lcd_read(u16 addr)
{
    u8 offset = addr - ADDR_LCD_START;
    u8 *ptr = (u8 *)lcd_get_context();
    return ptr[offset];
}
