#include <ppu_pipeline.h>
#include <ppu.h>
#include <lcd.h>
#include <bus.h>

bool window_visible()
{
    return LCDC_WIN_ENABLE &&
           LCD->win_x >= 0 &&
           LCD->win_x <= 166 &&
           LCD->win_y >= 0 &&
           LCD->win_y < YRES;
}

static void pixel_fifo_push(u32 value)
{
    fifo_entry *next = malloc(sizeof(fifo_entry));
    next->next = NULL;
    next->value = value;

    if (!PIXEL_FIFO->head)
    {
        PIXEL_FIFO->head = next;
        PIXEL_FIFO->tail = next;
    }
    else
    {
        PIXEL_FIFO->tail->next = next;
        PIXEL_FIFO->tail = next;
    }

    PIXEL_FIFO->size += 1;
}

static u32 pixel_fifo_pop()
{
    assert(PIXEL_FIFO->size > 0);

    fifo_entry *entry = PIXEL_FIFO->head;
    PIXEL_FIFO->head = entry->next;
    PIXEL_FIFO->tail = PIXEL_FIFO->head ? PIXEL_FIFO->tail : PIXEL_FIFO->head;
    PIXEL_FIFO->size -= 1;

    u32 value = entry->value;
    free(entry);

    return value;
}

static u32 fetch_sprite_pixels(int bit, u32 color, u8 bg_color)
{
    for (int i = 0; i < PPU->fetched_entry_count; i++)
    {
        const oam_entry *const entry = PPU->fetched_entries[i];

        int sp_x = entry->x - 8;
        sp_x += LCD->scroll_x % 8;

        if (sp_x + 8 < PFC->fifo_x)
            continue;

        int offset = PFC->fifo_x - sp_x;

        if (!BETWEEN(offset, 0, 7))
            continue;

        bit = entry->f_x_flip ? offset : 7 - offset;

        u8 hi = BIT(PFC->fetch_entry_data[(i * 2) + 0], bit) << 0;
        u8 lo = BIT(PFC->fetch_entry_data[(i * 2) + 1], bit) << 1;
        u8 index = hi | lo;

        if (index == 0)
            continue;

        if (entry->f_bgp == false || bg_color == 0)
        {
            u32 *palette_colors = entry->f_pn ? LCD->sp2_colors : LCD->sp1_colors;
            color = palette_colors[index];
            break;
        }
    }

    return color;
}

static bool pipeline_fifo_add()
{
    if (PIXEL_FIFO->size > 8)
        return false;

    if (PFC->fetch_x < 8 - (LCD->scroll_x % 8))
        return false;

    for (int bit = 7; bit >= 0; --bit)
    {
        const u8 hi = BIT(PFC->bgw_fetch_data[1 + 0], bit) << 0;
        const u8 lo = BIT(PFC->bgw_fetch_data[1 + 1], bit) << 1;
        const u8 index = hi | lo;
        const u32 color = LCD->bg_colors[LCDC_BGW_ENABLE ? index : 0];

        pixel_fifo_push(LCDC_OBJ_ENABLE ? fetch_sprite_pixels(bit, color, index) : color);
        PFC->fifo_x++;
    }

    return true;
}

static void pipeline_load_sprite_tile()
{
    PPU_FOREACH_LINE_SPRITE(line)
    {
        if (PPU->fetched_entry_count >= 3)
            break;

        int sp_x = line->entry->x - 8;
        sp_x += LCD->scroll_x % 8;

        bool nearby = false;
        nearby |= NEARBY_LIMIT(sp_x + 0, PFC->fetch_x, 8);
        nearby |= NEARBY_LIMIT(sp_x + 8, PFC->fetch_x, 8);

        if (nearby)
        {
            PPU->fetched_entries[PPU->fetched_entry_count] = line->entry;
            PPU->fetched_entry_count += 1;
        }
    }
}

static void pipeline_load_sprite_data(u8 offset)
{
    int cur_y = LCD->ly;
    u8 sprite_height = LCDC_OBJ_HEIGHT;

    for (int i = 0; i < PPU->fetched_entry_count; i++)
    {
        const oam_entry *const entry = PPU->fetched_entries[i];

        u8 tile_y = cur_y + 16;
        tile_y -= entry->y;
        tile_y *= 2;

        if (entry->f_y_flip)
            tile_y = ((sprite_height * 2) - 2) - tile_y;

        u8 tile_index = entry->tile;

        if (sprite_height == 16)
            tile_index &= ~(1);

        PFC->fetch_entry_data[(i * 2) + offset] = bus_read(ADDR_VRAM_START + (tile_index * 16) + tile_y + offset);
    }
}

void pipeline_load_window_tile()
{
    if (window_visible() == false)
        return;

    u8 window_x = LCD->win_x;
    u8 window_y = LCD->win_y;

    if (NEARBY_LIMIT(PFC->fetch_x + 7, window_x, YRES + 14))
    {
        if (NEARBY_LIMIT(LCD->ly, window_y, XRES))
        {
            u8 tile_y = PPU->window_line;
            tile_y >>= 3;

            u8 tile_x = PFC->fetch_x + 7 - window_x;
            tile_x >>= 3;

            PFC->bgw_fetch_data[0] = bus_read(LCDC_WIN_MAP_AREA + tile_y * 32 + tile_x);

            if (LCDC_BGW_DATA_AREA == 0x8800)
                PFC->bgw_fetch_data[0] += 128;
        }
    }
}

static void pipeline_fetch()
{
    switch (PFC->cur_fetch_state)
    {
    case FS_TILE:
    {
        PPU->fetched_entry_count = 0;

        if (LCDC_BGW_ENABLE)
        {
            u8 tile_y = PFC->map_y;
            tile_y >>= 3;

            u8 tile_x = PFC->map_x;
            tile_x >>= 3;

            PFC->bgw_fetch_data[0] = bus_read(LCDC_BG_MAP_AREA + tile_y * 32 + tile_x);

            if (LCDC_BGW_DATA_AREA == 0x8800)
                PFC->bgw_fetch_data[0] += 0x80;

            pipeline_load_window_tile();
        }

        if (LCDC_OBJ_ENABLE && PPU->line_sprites)
            pipeline_load_sprite_tile();

        PFC->cur_fetch_state = FS_DATA0;
        PFC->fetch_x += 8;
    }
    break;

    case FS_DATA0:
    {
        u8 data_index = PFC->bgw_fetch_data[0];
        u8 data_y = PFC->tile_y + 0;

        PFC->bgw_fetch_data[1] = bus_read(LCDC_BGW_DATA_AREA + data_index * 16 + data_y);
        pipeline_load_sprite_data(0);
        PFC->cur_fetch_state = FS_DATA1;
    }
    break;

    case FS_DATA1:
    {
        u8 data_index = PFC->bgw_fetch_data[0];
        u8 data_y = PFC->tile_y + 1;

        PFC->bgw_fetch_data[2] = bus_read(LCDC_BGW_DATA_AREA + data_index * 16 + data_y);
        pipeline_load_sprite_data(1);
        PFC->cur_fetch_state = FS_IDLE;
    }
    break;

    case FS_IDLE:
    {
        PFC->cur_fetch_state = FS_PUSH;
    }
    break;

    case FS_PUSH:
    {
        if (pipeline_fifo_add())
            PFC->cur_fetch_state = FS_TILE;
    }
    break;
    }
}

static void pipeline_push_pixel()
{
    if (PIXEL_FIFO->size > 8)
    {
        u32 pixel_data = pixel_fifo_pop();

        if (PFC->line_x >= (LCD->scroll_x % 8))
        {
            VIDEO_BUFFER_SET(PFC->pushed_x, LCD->ly, pixel_data);
            PFC->pushed_x++;
        }

        PFC->line_x++;
    }
}

void pipeline_process()
{
    PFC->map_y = LCD->ly + LCD->scroll_y;
    PFC->map_x = PFC->fetch_x + LCD->scroll_x;

    PFC->tile_y = PFC->map_y;
    PFC->tile_y %= 8;
    PFC->tile_y *= 2;

    if (!BIT(PPU->line_ticks, 0))
        pipeline_fetch();

    pipeline_push_pixel();
}

void pipeline_fifo_reset()
{
    while (PIXEL_FIFO->size)
        pixel_fifo_pop();

    assert(PIXEL_FIFO->size == 0);
    assert(PIXEL_FIFO->head == NULL);
    assert(PIXEL_FIFO->tail == NULL);
}