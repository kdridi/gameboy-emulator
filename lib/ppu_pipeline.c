#include <ppu_pipeline.h>
#include <ppu.h>
#include <lcd.h>
#include <bus.h>

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

    PIXEL_FIFO->size++;
}

static u32 pixel_fifo_pop(void)
{
    assert(PIXEL_FIFO->size > 0);
    assert(PIXEL_FIFO->head != NULL);
    assert(PIXEL_FIFO->tail != NULL);

    fifo_entry *entry = PIXEL_FIFO->head;
    u32 value = entry->value;

    PIXEL_FIFO->head = entry->next;
    PIXEL_FIFO->tail = PIXEL_FIFO->head ? PIXEL_FIFO->tail : PIXEL_FIFO->head;
    PIXEL_FIFO->size--;

    free(entry);

    return value;
}

static bool pipeline_fifo_add(void)
{
    if (PIXEL_FIFO->size > 8)
        return false;

    int x = PFC->fetch_x - (8 - (LCD->scroll_x % 8));

    for (int i = 0; i < 8; i++)
    {
        int bit = 7 - i;
        u8 hi = BIT(PFC->bgw_fetch_data[1], bit) << 0;
        u8 lo = BIT(PFC->bgw_fetch_data[2], bit) << 1;
        u32 color = LCD->bg_colors[hi | lo];

        if (x >= 0)
        {
            pixel_fifo_push(color);
            PFC->fifo_x++;
        }
    }

    return true;
}

static void pipeline_fetch(void)
{
    switch (PFC->cur_fetch_state)
    {
    case FS_TILE:
        if (LCDC_BGW_ENABLE)
        {
            PFC->bgw_fetch_data[0] = bus_read(LCDC_BG_MAP_AREA +
                                              (((PFC->map_x >> 3)) << (0 * 5)) +
                                              (((PFC->map_y >> 3)) << (1 * 5)));

            if (LCDC_BGW_DATA_AREA == 0x8800)
                PFC->bgw_fetch_data[0] += 128;
        }

        PFC->cur_fetch_state = FS_DATA0;
        PFC->fetch_x += 8;
        break;

    case FS_DATA0:
        PFC->bgw_fetch_data[1] = bus_read(LCDC_BGW_DATA_AREA + PFC->bgw_fetch_data[0] * 16 + PFC->tile_y + 0);
        PFC->cur_fetch_state = FS_DATA1;
        break;
    case FS_DATA1:
        PFC->bgw_fetch_data[2] = bus_read(LCDC_BGW_DATA_AREA + PFC->bgw_fetch_data[0] * 16 + PFC->tile_y + 1);
        PFC->cur_fetch_state = FS_IDLE;
        break;
    case FS_IDLE:
        PFC->cur_fetch_state = FS_PUSH;
        break;
    case FS_PUSH:
        if (pipeline_fifo_add())
            PFC->cur_fetch_state = FS_TILE;
        else
            PFC->cur_fetch_state = FS_PUSH;
        break;
    }
}

static void pipeline_push_pixel(void)
{
    if (PIXEL_FIFO->size > 8)
    {
        u32 pixel_data = pixel_fifo_pop();
        if (PFC->line_x >= (LCD->scroll_x % 8))
        {
            VIDEO_BUFFER[LCD->ly * XRES + PFC->pushed_x] = pixel_data;
            PFC->pushed_x++;
        }
        PFC->line_x++;
    }
}

void pipeline_process(void)
{
    PFC->map_x = LCD->scroll_x + PFC->fetch_x;
    PFC->map_y = LCD->scroll_y + LCD->ly;
    PFC->tile_y = PFC->map_y;
    PFC->tile_y %= 8;
    PFC->tile_y *= 2;

    if (!BIT(PPU->line_ticks, 1))
        pipeline_fetch();

    pipeline_push_pixel();
}

void pipeline_fifo_reset(void)
{
    while (PIXEL_FIFO->size)
        pixel_fifo_pop();

    assert(PIXEL_FIFO->size == 0);
    assert(PIXEL_FIFO->head == NULL);
    assert(PIXEL_FIFO->tail == NULL);
}
