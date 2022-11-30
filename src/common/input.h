#pragma once
#include <vector>

#define FDOV_FIRST_SCANCODE 512

// NOTE(dlb): Bit of a clever trick here to add some custom scancodes that
// represent mouse input to the SDL scancode enum so that we can treat mouse
// buttons the same as any other kind of key.
enum {
#if 0
    // Abstract modifier keys
    FDOV_SCANCODE_SHIFT,
    FDOV_SCANCODE_CTRL,
    FDOV_SCANCODE_ALT,
#endif

    // Mouse buttons
    FDOV_SCANCODE_MOUSE_LEFT = FDOV_FIRST_SCANCODE,
    FDOV_SCANCODE_MOUSE_RIGHT,
    FDOV_SCANCODE_MOUSE_MIDDLE,
    FDOV_SCANCODE_MOUSE_X1,
    FDOV_SCANCODE_MOUSE_X2,

    // Window "X" button (or Alt+F4, etc.)
    FDOV_SCANCODE_QUIT,

    FDOV_SCANCODE_COUNT
};

struct InputEvent {
    int  scancode {};
    bool down     {};  // true for KEYDOWN, false for KEYUP
};

typedef std::vector<InputEvent> InputQueue;
