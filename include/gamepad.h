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

    void gamepad_init();
    bool gamepad_button_sel();
    bool gamepad_dir_sel();
    void gamepad_set_sel(u8 value);

    gamepad_state *gamepad_get_state();
    u8 gamepad_get_output();

#if defined(__cplusplus)
}
#endif
