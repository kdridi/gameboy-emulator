#include <ppu_pipeline.h>
#include <ppu.h>
#include <lcd.h>
#include <bus.h>

static void pixel_fifo_push(u32 value)
{
    fifo_entry *next = malloc(sizeof(fifo_entry));
    next->next = NULL;
    next->value = value;

    if (!PFC->pixel_fifo.head)
    {
        // first entry...
        PFC->pixel_fifo.head = PFC->pixel_fifo.tail = next;
    }
    else
    {
        PFC->pixel_fifo.tail->next = next;
        PFC->pixel_fifo.tail = next;
    }

    PFC->pixel_fifo.size++;
}

static u32 pixel_fifo_pop()
{
    if (PFC->pixel_fifo.size <= 0)
    {
        fprintf(stderr, "ERR IN PIXEL FIFO!\n");
        exit(-8);
    }

    fifo_entry *popped = PFC->pixel_fifo.head;
    PFC->pixel_fifo.head = popped->next;
    PFC->pixel_fifo.size--;

    u32 val = popped->value;
    free(popped);

    return val;
}

static u32 fetch_sprite_pixels(int bit, u32 color, u8 bg_color)
{
    for (int i = 0; i < PPU->fetched_entry_count; i++)
    {
        int sp_x = (PPU->fetched_entries[i].x - 8) + ((LCD->scroll_x % 8));

        if (sp_x + 8 < PFC->fifo_x)
        {
            // past pixel point already...
            continue;
        }

        int offset = PFC->fifo_x - sp_x;

        if (offset < 0 || offset > 7)
        {
            // out of bounds..
            continue;
        }

        bit = (7 - offset);

        if (PPU->fetched_entries[i].f_x_flip)
        {
            bit = offset;
        }

        u8 hi = !!(PFC->fetch_entry_data[i * 2] & (1 << bit));
        u8 lo = !!(PFC->fetch_entry_data[(i * 2) + 1] & (1 << bit)) << 1;

        bool bg_priority = PPU->fetched_entries[i].f_bgp;

        if (!(hi | lo))
        {
            // transparent
            continue;
        }

        if (!bg_priority || bg_color == 0)
        {
            color = (PPU->fetched_entries[i].f_pn) ? LCD->sp2_colors[hi | lo] : LCD->sp1_colors[hi | lo];

            if (hi | lo)
            {
                break;
            }
        }
    }

    return color;
}

static bool pipeline_fifo_add()
{
    if (PFC->pixel_fifo.size > 8)
    {
        // fifo is full!
        return false;
    }

    int x = PFC->fetch_x - (8 - (LCD->scroll_x % 8));

    for (int i = 0; i < 8; i++)
    {
        int bit = 7 - i;
        u8 hi = !!(PFC->bgw_fetch_data[1] & (1 << bit));
        u8 lo = !!(PFC->bgw_fetch_data[2] & (1 << bit)) << 1;
        u32 color = LCD->bg_colors[hi | lo];

        if (!LCDC_BGW_ENABLE)
        {
            color = LCD->bg_colors[0];
        }

        if (LCDC_OBJ_ENABLE)
        {
            color = fetch_sprite_pixels(bit, color, hi | lo);
        }

        if (x >= 0)
        {
            pixel_fifo_push(color);
            PFC->fifo_x++;
        }
    }

    return true;
}

static void pipeline_load_sprite_tile()
{
    oam_line_entry *le = PPU->line_sprites;

    while (le)
    {
        int sp_x = (le->entry.x - 8) + (LCD->scroll_x % 8);

        if ((sp_x >= PFC->fetch_x && sp_x < PFC->fetch_x + 8) ||
            ((sp_x + 8) >= PFC->fetch_x && (sp_x + 8) < PFC->fetch_x + 8))
        {
            // need to add entry
            PPU->fetched_entries[PPU->fetched_entry_count++] = le->entry;
        }

        le = le->next;

        if (!le || PPU->fetched_entry_count >= 3)
        {
            // max checking 3 sprites on pixels
            break;
        }
    }
}

static void pipeline_load_sprite_data(u8 offset)
{
    int cur_y = LCD->ly;
    u8 sprite_height = LCDC_OBJ_HEIGHT;

    for (int i = 0; i < PPU->fetched_entry_count; i++)
    {
        u8 ty = ((cur_y + 16) - PPU->fetched_entries[i].y) * 2;

        if (PPU->fetched_entries[i].f_y_flip)
        {
            // flipped upside down...
            ty = ((sprite_height * 2) - 2) - ty;
        }

        u8 tile_index = PPU->fetched_entries[i].tile;

        if (sprite_height == 16)
        {
            tile_index &= ~(1); // remove last bit...
        }

        PFC->fetch_entry_data[(i * 2) + offset] =
            bus_read(0x8000 + (tile_index * 16) + ty + offset);
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
            PFC->bgw_fetch_data[0] = bus_read(LCDC_BG_MAP_AREA +
                                              (PFC->map_x / 8) +
                                              (((PFC->map_y / 8)) * 32));

            if (LCDC_BGW_DATA_AREA == 0x8800)
            {
                PFC->bgw_fetch_data[0] += 128;
            }
        }

        if (LCDC_OBJ_ENABLE && PPU->line_sprites)
        {
            pipeline_load_sprite_tile();
        }

        PFC->cur_fetch_state = FS_DATA0;
        PFC->fetch_x += 8;
    }
    break;

    case FS_DATA0:
    {
        PFC->bgw_fetch_data[1] = bus_read(LCDC_BGW_DATA_AREA +
                                          (PFC->bgw_fetch_data[0] * 16) +
                                          PFC->tile_y);

        pipeline_load_sprite_data(0);

        PFC->cur_fetch_state = FS_DATA1;
    }
    break;

    case FS_DATA1:
    {
        PFC->bgw_fetch_data[2] = bus_read(LCDC_BGW_DATA_AREA +
                                          (PFC->bgw_fetch_data[0] * 16) +
                                          PFC->tile_y + 1);

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
        {
            PFC->cur_fetch_state = FS_TILE;
        }
    }
    break;
    }
}

static void pipeline_push_pixel()
{
    if (PFC->pixel_fifo.size > 8)
    {
        u32 pixel_data = pixel_fifo_pop();

        if (PFC->line_x >= (LCD->scroll_x % 8))
        {
            PPU->video_buffer[PFC->pushed_x +
                              (LCD->ly * XRES)] = pixel_data;

            PFC->pushed_x++;
        }

        PFC->line_x++;
    }
}

void pipeline_process()
{
    PFC->map_y = (LCD->ly + LCD->scroll_y);
    PFC->map_x = (PFC->fetch_x + LCD->scroll_x);
    PFC->tile_y = ((LCD->ly + LCD->scroll_y) % 8) * 2;

    if (!(PPU->line_ticks & 1))
    {
        pipeline_fetch();
    }

    pipeline_push_pixel();
}

void pipeline_fifo_reset()
{
    while (PFC->pixel_fifo.size)
    {
        pixel_fifo_pop();
    }

    PFC->pixel_fifo.head = 0;
}