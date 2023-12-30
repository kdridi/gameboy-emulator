#pragma once

#include <common.h>

typedef struct
{
    u8 lcdc;           // LCD Control
    u8 lcds;           // LCD Status
    u8 scroll_y;       // Scroll Y
    u8 scroll_x;       // Scroll X
    u8 ly;             // LCD Y-Coordinate
    u8 ly_compare;     // LY Compare : this is the value that LY is compared to, when LY == LYC, the LYC=LY STAT interrupt is requested. It is useful for raster effects and scrolling.
    u8 dma;            // DMA Transfer and Start Address
    u8 bg_palette;     // BG Palette Data
    u8 obj_palette[2]; // Object Palette Data
    u8 win_y;          // Window Y Position
    u8 win_x;          // Window X Position

    u32 bg_colors[4];  // Background Color Palette
    u32 sp1_colors[4]; // Sprite 1 Color Palette
    u32 sp2_colors[4]; // Sprite 2 Color Palette
} lcd_context;

typedef enum
{
    MODE_HBLANK, //  204 clocks -   51 cycles -  1 scanline  : When the LCD controller is in mode 0, the CPU can access both the display RAM (8000h-9FFFh) and OAM (FE00h-FE9Fh)
    MODE_VBLANK, // 4560 clocks - 1140 cycles - 10 scanlines : When the LCD controller is in mode 1, the CPU can access both the display RAM (8000h-9FFFh) and OAM (FE00h-FE9Fh)
    MODE_OAM,    //   80 clocks -   20 cycles -  2 scanlines : When the LCD controller is in mode 2, the CPU can only access OAM (FE00h-FE9Fh)
    MODE_XFER,   //  172 clocks -   43 cycles -  2 scanlines : When the LCD controller is in mode 3, the CPU cannot access OAM (FE00h-FE9Fh) or VRAM (8000h-9FFFh)
} lcd_mode;

// clang-format off
#define LCD                 (lcd_get_context())

#define LCDC                (LCD->lcdc)
#define LCDC_BGW_ENABLE     ((BIT(LCDC, 0) == 1) ? true : false)
#define LCDC_OBJ_ENABLE     ((BIT(LCDC, 1) == 1) ? true : false)
#define LCDC_OBJ_HEIGHT     ((BIT(LCDC, 2) == 1) ? 16 : 8)
#define LCDC_BG_MAP_AREA    ((BIT(LCDC, 3) == 1) ? 0x9C00 : 0x9800)
#define LCDC_BGW_DATA_AREA  ((BIT(LCDC, 4) == 1) ? ADDR_VRAM_START : 0x8800)
#define LCDC_WIN_ENABLE     ((BIT(LCDC, 5) == 1) ? true : false)
#define LCDC_WIN_MAP_AREA   ((BIT(LCDC, 6) == 1) ? 0x9C00 : 0x9800)
#define LCDC_LCD_ENABLE     ((BIT(LCDC, 7) == 1) ? true : false)

#define LCDC_WIN_MAP_AREA_READ(tile_x, tile_y) (bus_read(LCDC_WIN_MAP_AREA + (tile_x) + ((tile_y) * 32)))
#define LCDC_WIN_MAP_AREA_READ_AT(at_x, at_y) LCDC_WIN_MAP_AREA_READ((at_x) >> 3, (at_y) >> 3)

#define LCDS                (LCD->lcds)
#define LCDS_MODE           ((lcd_mode)(LCDS & 0b11))
#define LCDS_MODE_SET(mode) (LCDS = (LCDS & 0b11111100) | mode)
#define LCDS_LYC            ((BIT(LCDS, 2) == 1) : true : false)
#define LCDS_LYC_SET(b)     BIT_SET(LCDS, 2, b)
#define LCDS_STAT_INT(src)  (LCDS & src)
// clang-format on

typedef enum
{
    SS_HBLANK = (1 << 3), // This bit is set when the LCD controller is in the H-Blank period (LCDSTAT Mode 0)
    SS_VBLANK = (1 << 4), // This bit is set when the LCD controller is in the V-Blank period (LCDSTAT Mode 1)
    SS_OAM = (1 << 5),    // This bit is set when the LCD controller is reading from OAM memory. The OAM is 160 bytes starting at 0xFE00.
    SS_LYC = (1 << 6),    // This bit is set during LY=LYC coincidence.
} stat_src;

#ifdef __cplusplus
extern "C"
{
#endif

    void lcd_init(void);
    lcd_context *lcd_get_context(void);

    void lcd_write(u16 addr, u8 value);
    u8 lcd_read(u16 addr);

#ifdef __cplusplus
}
#endif