#pragma once
#include "../facets/keymap.h"
#include "../common/button_state.h"
#include "../common/input.h"
#include "../common/message.h"

struct InputSystem {
public:
    void ProcessInput(
        double now,
        Depot &depot,
        const InputQueue &inputQueue);

private:
    ButtonState buttons[FDOV_SCANCODE_COUNT]{};

    void CheckHotkeys(
        double now,
        ButtonState buttons[FDOV_SCANCODE_COUNT],
        Keymap &keymap,
        MsgQueue &msgQueue);
};