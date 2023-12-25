#include <emu.h>
#include <cart.h>
#include <cpu.h>

#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

static emu_context ctx;

emu_context *emu_get_context(void)
{
    return &ctx;
}

void delay(u32 ms)
{
    SDL_Delay(ms);
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

    printf("Cart loaded...\n");

    SDL_Init(SDL_INIT_VIDEO);
    printf("SDL INIT\n");
    TTF_Init();
    printf("TTF INIT\n");

    cpu_init();

    ctx.running = true;
    ctx.paused = false;
    ctx.ticks = 0;

    while (ctx.running)
    {
        if (ctx.paused)
        {
            delay(10);
            continue;
        }

        if (!cpu_step())
        {
            printf("CPU Stopped\n");
            return -3;
        }

        ctx.ticks++;
    }

    return 0;
}

void emu_cycles(u64 cycles)
{
    // TODO: Implement
}