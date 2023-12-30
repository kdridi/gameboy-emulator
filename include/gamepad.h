#pragma once

#include <common.h>

#define GAMEPAD (gamepad_get_state())

typedef struct
{
    bool start;
    bool select;
    bool a;
    bool b;
    bool up;
    bool down;
    bool left;
    bool right;
} gamepad_state;

#if defined(__cplusplus)
extern "C"
{
#endif

    void gamepad_init(void);

    void gamepad_write(u8 value);
    u8 gamepad_read(void);

    gamepad_state *gamepad_get_state(void);

#if defined(__cplusplus)
}
#endif
