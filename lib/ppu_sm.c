#include <ppu_sm.h>
#include <ppu.h>
#include <lcd.h>
#include <interrupts.h>
#include <ppu_pipeline.h>
#include <cart.h>

bool window_visible(void);

static void increment_ly(void)
{
    if (window_visible() && LCD->ly >= LCD->win_y && LCD->ly < LCD->win_y + YRES)
        PPU->window_line++;

    LCD->ly++;

    if (LCD->ly == LCD->ly_compare)
    {
        LCDS_LYC_SET(1);
        if (LCDS_STAT_INT(SS_LYC))
            cpu_request_interrupt(IT_LCD_STAT);
    }
    else
        LCDS_LYC_SET(0);
}

static void load_line_sprites(void)
{
    int cur_y = LCD->ly;

    u8 sprite_height = LCDC_OBJ_HEIGHT;
    memset(PPU->line_entry_array, 0, sizeof(PPU->line_entry_array));

    for (int i = 0; i < 40; i++)
    {
        const oam_entry *const object_entry = PPU->oam_ram + i;

        // x = 0 means not visible...
        if (!object_entry->x)
            continue;

        // max 10 sprites per line...
        if (PPU->line_sprite_count >= 10)
            break;

        if (object_entry->y <= cur_y + 16 && object_entry->y + sprite_height > cur_y + 16)
        {
            // this sprite is on the current line.

            oam_line_entry *entry = PPU->line_entry_array + PPU->line_sprite_count;
            PPU->line_sprite_count += 1;

            entry->entry = object_entry;
            entry->next = NULL;

            if (PPU->line_sprites == NULL || PPU->line_sprites->entry->x > object_entry->x)
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
                if (le->entry->x > object_entry->x)
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

void ppu_mode_oam(void)
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

void ppu_mode_xfer(void)
{
    pipeline_process();

    if (PFC->pushed_x >= XRES)
    {
        pipeline_fifo_reset();
        LCDS_MODE_SET(MODE_HBLANK);
        if (LCDS_STAT_INT(SS_HBLANK))
            cpu_request_interrupt(IT_LCD_STAT);
    }
}

void ppu_mode_hblank(void)
{
    static u32 target_frame_time = 1000 / 60;
    static u32 prev_frame_time = 0;
    static u32 start_timer = 0;
    static u32 frame_count = 0;

    if (PPU->line_ticks >= TICKS_PER_LINE)
    {
        increment_ly();

        if (LCD->ly >= YRES)
        {
            LCDS_MODE_SET(MODE_VBLANK);
            cpu_request_interrupt(IT_VBLANK);
            if (LCDS_STAT_INT(SS_VBLANK))
                cpu_request_interrupt(IT_LCD_STAT);

            PPU->current_frame++;

            // calc FPS...
            u32 end = get_ticks();
            u32 frame_time = end - prev_frame_time;

            if (frame_time < target_frame_time)
                delay((target_frame_time - frame_time));

            if (end - start_timer >= 1000)
            {
                u32 fps = frame_count;
                start_timer = end;
                frame_count = 0;

                printf("FPS: %d\n", fps);

                if (cart_need_save())
                    cart_battery_save();
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

void ppu_mode_vblank(void)
{
    if (PPU->line_ticks >= TICKS_PER_LINE)
    {
        increment_ly();
        if (LCD->ly >= LINES_PER_FRAME)
        {
            LCDS_MODE_SET(MODE_OAM);
            LCD->ly = 0;
            PPU->window_line = 0;
        }
        PPU->line_ticks = 0;
    }
}
