#pragma once

#include <common.h>

#define LINES_PER_FRAME 154
#define TICKS_PER_LINE 456
#define YRES 144
#define XRES 160

typedef struct
{
    u8 x, y, tile;

    unsigned f_cgb_pn : 3;        // Palette number **CGB Mode Only** (OBP0-7)
    unsigned f_cgb_vram_bank : 1; // Tile VRAM Bank **CGB Mode Only** (0=Bank 0, 1=Bank 1)
    unsigned f_pn : 1;            // Palette number **DMG Mode Only** (0=OBP0, 1=OBP1)
    unsigned f_x_flip : 1;        // Flip horizontally (0=Normal, 1=Mirror horizontally)
    unsigned f_y_flip : 1;        // Flip vertically (0=Normal, 1=Mirror vertically)
    unsigned f_bgp : 1;           // Background and window over OBJ (0=No, 1=BG and Window colors 1-3 over the OBJ)
} oam_entry;

typedef struct
{
    oam_entry oam_ram[40];
    u8 vram[0x2000];

    u32 current_frame;
    u32 line_ticks;
    u32 *video_buffer;
} ppu_context;

#ifdef __cplusplus
extern "C"
{
#endif

    void ppu_init(void);
    void ppu_tick(void);

    void ppu_oam_write(u16 address, u8 value);
    u8 ppu_oam_read(u16 address);

    void ppu_vram_write(u16 address, u8 value);
    u8 ppu_vram_read(u16 address);

    ppu_context *ppu_get_context(void);

#ifdef __cplusplus
}
#endif