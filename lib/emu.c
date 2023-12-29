#include <emu.h>
#include <cart.h>
#include <cpu.h>
#include <ui.h>
#include <timer.h>
#include <dma.h>
#include <ppu.h>

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

static emu_context ctx;

emu_context *emu_get_context(void)
{
    return &ctx;
}

void emu_init(void)
{
    timer_init();
    cpu_init();
    ppu_init();

    ctx.running = true;
    ctx.paused = false;
    ctx.ticks = 0;
}

void *cpu_run(void *data)
{
    ((void)data);

    while (ctx.running)
    {
        if (ctx.paused)
        {
            delay(10);
            continue;
        }

        if (emu_get_context()->ticks >= 0x000201E0)
        {
            // printf("BREAK\n");
        }

        if (!cpu_step())
        {
            printf("CPU Stopped\n");
            return NULL;
        }
    }

    return NULL;
}

int emu_run(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: %s <rom>\n", argv[0]);
        return -1;
    }

    if (!cart_load(argv[1]))
    {
        printf("Failed to load ROM file: %s\n", argv[1]);
        return -2;
    }

    printf("Cart loaded..\n");

    emu_init();
    ui_init();

    pthread_t cpu_thread;
    if (pthread_create(&cpu_thread, NULL, cpu_run, NULL))
    {

        printf("Failed to create CPU thread\n");
        return 84;
    }

    u32 prev_frame = 0;

    while (!ctx.die)
    {
        usleep(1000);
        ui_handle_events();

        if (prev_frame != ppu_get_context()->current_frame)
        {
            prev_frame = ppu_get_context()->current_frame;
            ui_update();
        }
    }

    return 0;
}

void emu_cycles(u64 cpu_cycles)
{
    for (u64 i = 0; i < cpu_cycles; i++)
    {
        for (u8 n = 0; n < 4; n++)
        {
            ctx.ticks++;
            timer_tick();
            ppu_tick();
        }

        dma_tick();
    }
}
