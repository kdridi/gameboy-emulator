#include <gamepad.h>
#include <string.h>

#undef GAMEPAD
#define GAMEPAD ctx.controller

typedef struct
{
    bool select_action;
    bool select_direction;
    gamepad_state controller;
} gamepad_context;

static gamepad_context ctx = {0};

void gamepad_write(u8 value)
{
    ctx.select_action = BIT(value, 5) == 0;
    ctx.select_direction = BIT(value, 4) == 0;
    assert(!ctx.select_action || !ctx.select_direction);
}

gamepad_state *gamepad_get_state()
{
    return &ctx.controller;
}

u8 gamepad_read()
{
    u8 output = 0b11001111;

    BIT_SET(output, 4, !ctx.select_action);
    if (ctx.select_action)
    {
        BIT_SET(output, 0, !GAMEPAD.a);
        BIT_SET(output, 1, !GAMEPAD.b);
        BIT_SET(output, 2, !GAMEPAD.select);
        BIT_SET(output, 3, !GAMEPAD.start);
    }

    BIT_SET(output, 5, !ctx.select_direction);
    if (ctx.select_direction)
    {
        BIT_SET(output, 0, !GAMEPAD.right);
        BIT_SET(output, 1, !GAMEPAD.left);
        BIT_SET(output, 2, !GAMEPAD.up);
        BIT_SET(output, 3, !GAMEPAD.down);
    }

    return output;
}