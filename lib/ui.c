#include <ui.h>
#include <emu.h>

#include <stdio.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

SDL_Window *sdlWindow;
SDL_Renderer *sdlRenderer;
SDL_Texture *sdlTexture;
SDL_Surface *screen;

void delay(u32 ms)
{
    SDL_Delay(ms);
}

void ui_init(void)
{
    SDL_Init(SDL_INIT_VIDEO);
    printf("SDL INIT\n");
    TTF_Init();
    printf("TTF INIT\n");

    SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &sdlWindow, &sdlRenderer);
}

void ui_handle_events(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event) > 0)
    {
        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE)
            emu_get_context()->die = true;

        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
            emu_get_context()->die = true;
    }
}
