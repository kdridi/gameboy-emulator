#pragma once

#include <common.h>

typedef struct
{
    u8 lcdc;           // LCD Control
    u8 lcds;           // LCD Status
    u8 scroll_y;       // Scroll Y
    u8 scroll_x;       // Scroll X
    u8 ly;             // LCD Y-Coordinate
    u8 ly_compare;     // LY Compare
    u8 dma;            // DMA Transfer and Start Address
    u8 bg_palette;     // BG Palette Data
    u8 obj_palette[2]; // Object Palette Data
    u8 win_y;          // Window Y Position
    u8 win_x;          // Window X Position

    u8 bg_colors[4];  // Background Color Palette
    u8 sp1_colors[4]; // Sprite 1 Color Palette
    u8 sp2_colors[4]; // Sprite 2 Color Palette
} lcd_context;

lcd_context *lcd_get_context(void);

typedef enum
{
    MODE_HBLANK, //  204 clocks -   51 cycles -  1 scanline  : When the LCD controller is in mode 0, the CPU can access both the display RAM (8000h-9FFFh) and OAM (FE00h-FE9Fh)
    MODE_VBLANK, // 4560 clocks - 1140 cycles - 10 scanlines : When the LCD controller is in mode 1, the CPU can access both the display RAM (8000h-9FFFh) and OAM (FE00h-FE9Fh)
    MODE_OAM,    //   80 clocks -   20 cycles -  2 scanlines : When the LCD controller is in mode 2, the CPU can only access OAM (FE00h-FE9Fh)
    MODE_XFER,   //  172 clocks -   43 cycles -  2 scanlines : When the LCD controller is in mode 3, the CPU cannot access OAM (FE00h-FE9Fh) or VRAM (8000h-9FFFh)
} lcd_mode;

#define LCDC_BGW_ENABLE ((BIT(lcd_get_context()->lcdc, 0) == 1) : true : false)
#define LCDC_OBJ_ENABLE ((BIT(lcd_get_context()->lcdc, 1) == 1) : true : false)
#define LCDC_OBJ_HEIGHT ((BIT(lcd_get_context()->lcdc, 2) == 1) : 16 : 8)
#define LCDC_BG_MAP_AREA ((BIT(lcd_get_context()->lcdc, 3) == 1) : 0x9C00 : 0x9800)
#define LCDC_BGW_DATA_AREA ((BIT(lcd_get_context()->lcdc, 4) == 1) : 0x8000 : 0x8800)
#define LCDC_WIN_ENABLE ((BIT(lcd_get_context()->lcdc, 5) == 1) : true : false)
#define LCDC_WIN_MAP_AREA ((BIT(lcd_get_context()->lcdc, 6) == 1) : 0x9C00 : 0x9800)
#define LCDC_LCD_ENABLE ((BIT(lcd_get_context()->lcdc, 7) == 1) : true : false)

#define LCDS_MODE ((lcd_mode)(lcd_get_context()->lcds & 0b11))
#define LCDS_MODE_SET(mode) (lcd_get_context()->lcds = (lcd_get_context()->lcds & 0b11111100) | mode)

#define LCDS_LYC ((BIT(lcd_get_context()->lcds, 2) == 1) : true : false)
#define LCDS_LYC_SET(b) BIT_SET(lcd_get_context()->lcds, 2, b)

typedef enum
{
    SS_HBLANK = (1 << 3),
    SS_VBLANK = (1 << 4),
    SS_OAM = (1 << 5),
    SS_LYC = (1 << 6),
} stat_src;

#define LCDS_STAT_INT(src) (lcd_get_context()->lcds & src)

#ifdef __cplusplus
extern "C"
{
#endif

    void lcd_init(void);

    void lcd_write(u16 addr, u8 value);
    u8 lcd_read(u16 addr);

#ifdef __cplusplus
}
#endif