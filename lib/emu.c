#include <emu.h>
#include <cart.h>
#include <cpu.h>
#include <ui.h>

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

static emu_context ctx;

emu_context *emu_get_context(void)
{
    return &ctx;
}

void *cpu_run(void *data)
{
    ((void)data);

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
            return NULL;
        }

        ctx.ticks++;
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

    printf("Cart loaded...\n");

    ui_init();

    pthread_t cpu_thread;
    if (pthread_create(&cpu_thread, NULL, cpu_run, NULL))
    {

        printf("Failed to create CPU thread\n");
        return 84;
    }

    while (!ctx.die)
    {
        usleep(1000);
        ui_handle_events();
    }

    return 0;
}

void emu_cycles(u64 cycles)
{
    // TODO: Implement
}