#pragma once
#include "../facets/keymap.h"
#include "../common/button_state.h"
#include "../common/input.h"
#include "../common/message.h"

struct InputEvent {
    int  scancode {};
    bool down     {};  // true for KEYDOWN, false for KEYUP
};

typedef std::vector<InputEvent> InputQueue;

struct InputSystem {
public:
    void PushInputEvent(int scancode, bool down);
    void ProcessInputQueue(Depot &depot);

private:
    InputQueue inputQueue{};
    ButtonState buttons[FDOV_SCANCODE_COUNT]{};

    /*void CheckHotkeys(
        double now,
        ButtonState buttons[FDOV_SCANCODE_COUNT],
        Keymap &keymap,
        MsgQueue &msgQueue);*/
};