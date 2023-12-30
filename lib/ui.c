#include <ui.h>
#include <emu.h>
#include <bus.h>
#include <ppu.h>
#include <gamepad.h>

#include <stdio.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

SDL_Window *sdlWindow;
SDL_Renderer *sdlRenderer;
SDL_Texture *sdlTexture;
SDL_Surface *screen;

SDL_Window *sdlDebugWindow;
SDL_Renderer *sdlDebugRenderer;
SDL_Texture *sdlDebugTexture;
SDL_Surface *debugScreen;

void delay(u32 ms)
{
    SDL_Delay(ms);
}

u32 get_ticks()
{
    return SDL_GetTicks();
}

static u32 tile_colors[] = {
    COLOR0,
    COLOR1,
    COLOR2,
    COLOR3,
};

void display_tile(SDL_Surface *surface, u16 startLocation, u16 tileNum, int x, int y)
{
    SDL_Rect rc;

    for (int tileY = 0; tileY < 16; tileY += 2)
    {
        u8 b1 = bus_read(startLocation + (tileNum * 16) + tileY + 0);
        u8 b2 = bus_read(startLocation + (tileNum * 16) + tileY + 1);

        for (int bit = 7; bit >= 0; bit--)
        {
            u8 hi = BIT(b1, bit);
            u8 lo = BIT(b2, bit);

            u8 color = (hi << 1) | lo;

            rc.x = x + ((7 - bit) * DEBUG_SCALE);
            rc.y = y + (tileY / 2) * DEBUG_SCALE;
            rc.w = DEBUG_SCALE;
            rc.h = DEBUG_SCALE;

            u32 color32 = tile_colors[color];

            SDL_FillRect(surface, &rc, color32);
        }
    }
}

void update_debug_window()
{
    int status = 0;
    int xDraw = 0;
    int yDraw = 0;
    int tileNum = 0;

    SDL_Rect rect = {.x = 0, .y = 0, .w = debugScreen->w, .h = debugScreen->h};
    status = SDL_FillRect(debugScreen, &rect, 0xFF111111);
    assert(status == 0);

    u16 addr = ADDR_VRAM_START;
    // 384 tiles = 24 rows * 16 columns
    for (u8 row = 0; row < 24; row++)
    {
        for (u8 col = 0; col < 16; col++)
        {
            display_tile(debugScreen, addr, tileNum, xDraw + (col * DEBUG_SCALE), yDraw + (row * DEBUG_SCALE));
            xDraw += 8 * DEBUG_SCALE;
            tileNum++;
        }

        xDraw = 0;
        yDraw += 8 * DEBUG_SCALE;
    }

    status = SDL_UpdateTexture(sdlDebugTexture, NULL, debugScreen->pixels, debugScreen->pitch);
    assert(status == 0);

    status = SDL_RenderClear(sdlDebugRenderer);
    assert(status == 0);

    status = SDL_RenderCopy(sdlDebugRenderer, sdlDebugTexture, NULL, NULL);
    assert(status == 0);

    SDL_RenderPresent(sdlDebugRenderer);
}

void ui_update()
{
    int status = 0;

    SDL_Rect rc;
    rc.x = rc.y = 0;
    rc.h = rc.w = DEBUG_SCALE;

    for (int y = 0; y < YRES; y++, rc.y += DEBUG_SCALE, rc.x = 0)
        for (int x = 0; x < XRES; x++, rc.x += DEBUG_SCALE)
            SDL_FillRect(screen, &rc, VIDEO_BUFFER_GET(x, y));

    status = SDL_UpdateTexture(sdlTexture, NULL, screen->pixels, screen->pitch);
    assert(status == 0);

    status = SDL_RenderClear(sdlRenderer);
    assert(status == 0);

    status = SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
    assert(status == 0);

    SDL_RenderPresent(sdlRenderer);

    update_debug_window();
}

void ui_init(void)
{
    int status;

    status = SDL_Init(SDL_INIT_VIDEO);
    assert(status == 0);
    printf("SDL INIT\n");

    status = TTF_Init();
    assert(status == 0);
    printf("TTF INIT\n");

    status = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &sdlWindow, &sdlRenderer);
    assert(status == 0);

    screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
                                  0x00FF0000,
                                  0x0000FF00,
                                  0x000000FF,
                                  0xFF000000);
    assert(screen != NULL);

    sdlTexture = SDL_CreateTexture(sdlRenderer,
                                   SDL_PIXELFORMAT_ARGB8888,
                                   SDL_TEXTUREACCESS_STREAMING,
                                   SCREEN_WIDTH, SCREEN_HEIGHT);
    assert(sdlTexture != NULL);

    status = SDL_CreateWindowAndRenderer(DEBUG_SCREEN_WIDTH, DEBUG_SCREEN_HEIGHT, 0, &sdlDebugWindow, &sdlDebugRenderer);
    assert(status == 0);

    debugScreen = SDL_CreateRGBSurface(0,
                                       DEBUG_SCREEN_WIDTH + DEBUG_SCREEN_WIDTH / 8,
                                       DEBUG_SCREEN_HEIGHT + DEBUG_SCREEN_HEIGHT / 4,
                                       32,
                                       0x00FF0000,
                                       0x0000FF00,
                                       0x000000FF,
                                       0xFF000000);
    assert(debugScreen != NULL);

    sdlDebugTexture = SDL_CreateTexture(sdlDebugRenderer,
                                        SDL_PIXELFORMAT_ARGB8888,
                                        SDL_TEXTUREACCESS_STREAMING,
                                        DEBUG_SCREEN_WIDTH + DEBUG_SCREEN_WIDTH / 8,
                                        DEBUG_SCREEN_HEIGHT + DEBUG_SCREEN_HEIGHT / 4);
    assert(sdlDebugTexture != NULL);

    int x, y;
    SDL_GetWindowPosition(sdlWindow, &x, &y);
    SDL_SetWindowPosition(sdlDebugWindow, x + SCREEN_WIDTH + 10, y);
}

void ui_on_key(bool down, u32 key_code)
{
    // clang-format off
    switch (key_code)
    {
        case SDLK_z:      GAMEPAD->b      = down; break;
        case SDLK_x:      GAMEPAD->a      = down; break;
        case SDLK_RETURN: GAMEPAD->start  = down; break;
        case SDLK_TAB:    GAMEPAD->select = down; break;
        case SDLK_UP:     GAMEPAD->up     = down; break;
        case SDLK_DOWN:   GAMEPAD->down   = down; break;
        case SDLK_LEFT:   GAMEPAD->left   = down; break;
        case SDLK_RIGHT:  GAMEPAD->right  = down; break;
    }
    // clang-format on
}

void ui_handle_events(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event) > 0)
    {
        if (event.type == SDL_KEYDOWN)
            ui_on_key(true, event.key.keysym.sym);

        if (event.type == SDL_KEYUP)
            ui_on_key(false, event.key.keysym.sym);

        if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE)
            EMU->die = true;

        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
            EMU->die = true;
    }
}
