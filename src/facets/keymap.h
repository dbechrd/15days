#pragma once
#include "facet.h"
#include "../common/button_state.h"
#include "../common/message.h"
#include <vector>

enum HotkeyFlags {
    Hotkey_Press   = 1 << 0,  // trigger when all keys first pressed
    Hotkey_Hold    = 1 << 1,  // trigget when all keys held down
    Hotkey_Release = 1 << 2,  // trigger when all keys released after being active
    Hotkey_Handled = 1 << 3,  // allow already handled keys to trigger the hotkey
};

struct KeymapHotkey {
    int         keys    [3]{};  // key scancodes that make up the hotkey (0 if unused)
    int         flags   {};     // (HotkeyFlags) when to treat the hotkey as active
    MsgType     msgType {};     // the command that this hotkey triggers
    ButtonState state   {};     // tracks hotkey state as if it were a button

    inline KeymapHotkey(int key0, int key1, int key2, int flags, MsgType msgType) {
        this->keys[0] = key0;
        this->keys[1] = key1;
        this->keys[2] = key2;
        this->flags = flags;
        this->msgType = msgType;
    }
};
struct Keymap : public Facet {
    std::vector<KeymapHotkey> hotkeys {};
};