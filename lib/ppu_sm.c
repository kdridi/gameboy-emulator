#include <ppu_sm.h>
#include <ppu.h>
#include <lcd.h>
#include <interrupts.h>
#include <ppu_pipeline.h>

static void increment_ly()
{
    LCD->ly++;

    if (LCD->ly == LCD->ly_compare)
    {
        LCDS_LYC_SET(1);

        if (LCDS_STAT_INT(SS_LYC))
        {
            cpu_request_interrupt(IT_LCD_STAT);
        }
    }
    else
    {
        LCDS_LYC_SET(0);
    }
}

static void load_line_sprites()
{
    int cur_y = LCD->ly;

    u8 sprite_height = LCDC_OBJ_HEIGHT;
    memset(PPU->line_entry_array, 0,
           sizeof(PPU->line_entry_array));

    for (int i = 0; i < 40; i++)
    {
        oam_entry e = PPU->oam_ram[i];

        if (!e.x)
        {
            // x = 0 means not visible...
            continue;
        }

        if (PPU->line_sprite_count >= 10)
        {
            // max 10 sprites per line...
            break;
        }

        if (e.y <= cur_y + 16 && e.y + sprite_height > cur_y + 16)
        {
            // this sprite is on the current line.

            oam_line_entry *entry = &PPU->line_entry_array[PPU->line_sprite_count++];

            entry->entry = e;
            entry->next = NULL;

            if (!PPU->line_sprites ||
                PPU->line_sprites->entry.x > e.x)
            {
                entry->next = PPU->line_sprites;
                PPU->line_sprites = entry;
                continue;
            }

            // do some sorting...

            oam_line_entry *le = PPU->line_sprites;
            oam_line_entry *prev = le;

            while (le)
            {
                if (le->entry.x > e.x)
                {
                    prev->next = entry;
                    entry->next = le;
                    break;
                }

                if (!le->next)
                {
                    le->next = entry;
                    break;
                }

                prev = le;
                le = le->next;
            }
        }
    }
}

void ppu_mode_oam()
{
    if (PPU->line_ticks >= 80)
    {
        LCDS_MODE_SET(MODE_XFER);

        PFC->cur_fetch_state = FS_TILE;
        PFC->line_x = 0;
        PFC->fetch_x = 0;
        PFC->pushed_x = 0;
        PFC->fifo_x = 0;
    }

    if (PPU->line_ticks == 1)
    {
        // read oam on the first tick only...
        PPU->line_sprites = 0;
        PPU->line_sprite_count = 0;

        load_line_sprites();
    }
}

void ppu_mode_xfer()
{
    pipeline_process();

    if (PFC->pushed_x >= XRES)
    {
        pipeline_fifo_reset();

        LCDS_MODE_SET(MODE_HBLANK);

        if (LCDS_STAT_INT(SS_HBLANK))
        {
            cpu_request_interrupt(IT_LCD_STAT);
        }
    }
}

void ppu_mode_vblank()
{
    if (PPU->line_ticks >= TICKS_PER_LINE)
    {
        increment_ly();

        if (LCD->ly >= LINES_PER_FRAME)
        {
            LCDS_MODE_SET(MODE_OAM);
            LCD->ly = 0;
        }

        PPU->line_ticks = 0;
    }
}

static u32 target_frame_time = 1000 / 60;
static long prev_frame_time = 0;
static long start_timer = 0;
static long frame_count = 0;

void ppu_mode_hblank()
{
    if (PPU->line_ticks >= TICKS_PER_LINE)
    {
        increment_ly();

        if (LCD->ly >= YRES)
        {
            LCDS_MODE_SET(MODE_VBLANK);

            cpu_request_interrupt(IT_VBLANK);

            if (LCDS_STAT_INT(SS_VBLANK))
            {
                cpu_request_interrupt(IT_LCD_STAT);
            }

            PPU->current_frame++;

            // calc FPS...
            u32 end = get_ticks();
            u32 frame_time = end - prev_frame_time;

            if (frame_time < target_frame_time)
            {
                delay((target_frame_time - frame_time));
            }

            if (end - start_timer >= 1000)
            {
                u32 fps = frame_count;
                start_timer = end;
                frame_count = 0;

                printf("FPS: %d\n", fps);
            }

            frame_count++;
            prev_frame_time = get_ticks();
        }
        else
        {
            LCDS_MODE_SET(MODE_OAM);
        }

        PPU->line_ticks = 0;
    }
}
