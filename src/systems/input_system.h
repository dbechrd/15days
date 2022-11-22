#pragma once
#include "../common/input.h"
#include "../common/message.h"
#include "../facets/depot.h"

struct InputSystem {
public:
    void ProcessInput(
        double now,
        const InputQueue &inputQueue,
        Keymap &keymap,
        MsgQueue &msgQueue);

private:
    ButtonState buttons[FDOV_SCANCODE_COUNT]{};

    void CheckHotkeys(
        double now,
        ButtonState buttons[FDOV_SCANCODE_COUNT],
        Keymap &keymap,
        MsgQueue &msgQueue);
};