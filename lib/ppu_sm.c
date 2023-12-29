#include <ppu_sm.h>
#include <ppu.h>
#include <lcd.h>
#include <interrupts.h>
#include <ppu_pipeline.h>

static void increment_ly(void)
{
    LCD->ly++;
    if (LCD->ly == LCD->ly_compare)
    {
        LCDS_LYC_SET(1);

        if (LCDS_STAT_INT(SS_LYC))
            cpu_request_interrupt(IT_LCD_STAT);
    }
    else
    {
        LCDS_LYC_SET(0);
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
    static u32 target_frame_time = 15;
    static u32 prev_frame_time = 0;
    static u32 start_timer = 0;
    static u32 frame_count = 0;

    if (PPU->line_ticks >= 80 + 172 + 204) // TICKS_PER_LINE = 80 + 172 + 204 = 456
    {
        increment_ly();
        if (LCD->ly >= YRES)
        {
            LCDS_MODE_SET(MODE_VBLANK);
            cpu_request_interrupt(IT_VBLANK);

            if (LCDS_STAT_INT(SS_VBLANK))
                cpu_request_interrupt(IT_LCD_STAT);

            PPU->current_frame++;

            // calc FPS
            u32 end = get_ticks();
            u32 frame_time = end - prev_frame_time;

            if (frame_time < target_frame_time)
                delay(target_frame_time - frame_time);

            if (end - start_timer >= 1000)
            {
                u32 fps = frame_count;
                start_timer = end;
                frame_count = 0;

                printf("FPS: %u\n", fps);
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
    if (PPU->line_ticks >= 80 + 172 + 204) // TICKS_PER_LINE = 80 + 172 + 204 = 456
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
