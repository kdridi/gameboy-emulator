#pragma once

#include <common.h>

#define PPU (ppu_get_context())
#define PPU_FOREACH_LINE_SPRITE(__line) \
    for (oam_line_entry *__line = PPU->line_sprites; __line != NULL; __line = __line->next)

#define PFC (&((PPU)->pfc))

#define PIXEL_FIFO (&((PFC)->pixel_fifo))

#define VIDEO_BUFFER ((PPU)->video_buffer)
#define VIDEO_BUFFER_GET(x, y) (VIDEO_BUFFER[(x) + ((y) * XRES)])
#define VIDEO_BUFFER_SET(x, y, value) (VIDEO_BUFFER[(x) + ((y) * XRES)] = (value))

typedef enum
{
    FS_TILE,
    FS_DATA0,
    FS_DATA1,
    FS_IDLE,
    FS_PUSH,
} fetch_state;

typedef struct _fifo_entry
{
    struct _fifo_entry *next;
    u32 value; // 32 bit color value.
} fifo_entry;

typedef struct
{
    fifo_entry *head;
    fifo_entry *tail;
    u32 size;
} fifo;

typedef struct
{
    fetch_state cur_fetch_state;
    fifo pixel_fifo;
    u8 line_x;
    u8 pushed_x;
    u8 fetch_x;
    u8 bgw_fetch_data[3];
    u8 fetch_entry_data[6]; // oam data..
    u8 map_y;
    u8 map_x;
    u8 tile_y;
    u8 fifo_x;
} pixel_fifo_context;

typedef struct
{
    u8 y;
    u8 x;
    u8 tile;

    u8 f_cgb_pn : 3;        // Palette number **CGB Mode Only** (OBP0-7)
    u8 f_cgb_vram_bank : 1; // Tile VRAM Bank **CGB Mode Only** (0=Bank 0, 1=Bank 1)
    u8 f_pn : 1;            // Palette number **DMG Mode Only** (0=OBP0, 1=OBP1)
    u8 f_x_flip : 1;        // Flip horizontally (0=Normal, 1=Mirror horizontally)
    u8 f_y_flip : 1;        // Flip vertically (0=Normal, 1=Mirror vertically)
    u8 f_bgp : 1;           // Background and window over OBJ (0=No, 1=BG and Window colors 1-3 over the OBJ)
} oam_entry;

typedef struct _oam_line_entry
{
    const oam_entry *entry;
    struct _oam_line_entry *next;
} oam_line_entry;

typedef struct
{
    oam_entry oam_ram[40];
    u8 vram[0x2000];

    pixel_fifo_context pfc;

    u8 line_sprite_count;                // 0 to 10 sprites.
    oam_line_entry *line_sprites;        // linked list of current sprites on line.
    oam_line_entry line_entry_array[10]; // memory to use for list.

    u8 fetched_entry_count;
    const oam_entry *fetched_entries[3]; // entries fetched during pipeline.
    u8 window_line;

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
