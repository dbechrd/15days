#include "input_system.h"
#include "../facets/depot.h"

void InputSystem::PushInputEvent(int scancode, bool down)
{
    InputEvent inputEvent{};
    inputEvent.scancode = scancode;
    inputEvent.down = down;
    inputQueue.push_back(inputEvent);
}

// Return true if you want to skip further processing of hotkeys
bool CheckHotkeysDebug(Depot &depot, const InputEvent &e)
{
    SDL_Keycode keycode = SDL_GetKeyFromScancode((SDL_Scancode)e.scancode);
    SDL_GetKeyName(keycode);

    switch (e.scancode) {
        case SDL_SCANCODE_K: {
            if (e.down) {
                static int kern = 0;
                printf("kern: %d\n", kern);
                for (Font &font : depot.font) {
                    TTF_SetFontKerning(font.ttf_font, kern);
                }
                kern = !kern;
            }
            break;
        }
        case SDL_SCANCODE_N: {
            depot.textSystem.PushUpdateNarrator(depot, {}, C255(COLOR_WHITE),
                C_RED     "Red"
                C_GREEN   " Green"
                C_BLUE    " Blue"
                C_CYAN    " Cyan"
                C_MAGENTA " Magenta"
                C_YELLOW  " Yellow"
                C_WHITE   " White"
            );
            break;
        }
        default: break;
    }

    return false;
}

void CheckHotkeys(double now, ButtonState buttons[FDOV_SCANCODE_COUNT],
                  Keymap &keymap, MsgQueue &msgQueue)
{
    // Determine if that event caused any hotkeys to trigger.
    // If so, queue a command.
    for (KeymapHotkey &hotkey : keymap.hotkeys) {
        int k0 = hotkey.keys[0];
        int k1 = hotkey.keys[1];
        int k2 = hotkey.keys[2];

        // Ignore unbound hotkeys
        if (!k0) {
            assert(!k1);
            assert(!k2);
            continue;
        }

        bool includehandled = hotkey.flags & Hotkey_Handled;

#if 0
        bool shift = buttons[FDOV_SCANCODE_SHIFT].Active(includehandled);
        bool ctrl = buttons[FDOV_SCANCODE_CTRL].Active(includehandled);
        bool alt = buttons[FDOV_SCANCODE_ALT].Active(includehandled);
#else
        bool lShift = buttons[SDL_SCANCODE_LSHIFT].Active(includehandled);
        bool rShift = buttons[SDL_SCANCODE_RSHIFT].Active(includehandled);
        bool lCtrl = buttons[SDL_SCANCODE_LCTRL].Active(includehandled);
        bool rCtrl = buttons[SDL_SCANCODE_RCTRL].Active(includehandled);
        bool lAlt = buttons[SDL_SCANCODE_LALT].Active(includehandled);
        bool rAlt = buttons[SDL_SCANCODE_RALT].Active(includehandled);
        bool shift = lShift || rShift;
        bool ctrl = lCtrl || rCtrl;
        bool alt = lAlt || rAlt;
#endif
        bool modAny = hotkey.modMask == HotkeyMod_Any;
        bool modShift = shift == ((hotkey.modMask & HotkeyMod_Shift) > 0);
        bool modCtrl  = ctrl  == ((hotkey.modMask & HotkeyMod_Ctrl ) > 0);
        bool modAlt   = alt   == ((hotkey.modMask & HotkeyMod_Alt  ) > 0);
        bool modMatch = modAny || (modShift && modCtrl && modAlt);

        bool key0 = buttons[k0].Active(includehandled);
        bool key1 = !k1 || buttons[k1].Active(includehandled);
        bool key2 = !k2 || buttons[k2].Active(includehandled);
        bool keyMatch = key0 && key1 && key2;

        bool active = modMatch && keyMatch;

        hotkey.state.Set(active, now);

        bool pressed = (hotkey.flags & Hotkey_Press) && hotkey.state.Pressed();
        bool released = (hotkey.flags & Hotkey_Release) && hotkey.state.Released();
        if (pressed || released) {
            // NOTE: It's fine if these set buttons[0].handled to true
            buttons[k0].handled = true;
            buttons[k1].handled = true;
            buttons[k2].handled = true;
            Message msg{};
            msg.uid = keymap.uid;
            msg.type = hotkey.msgType;
            msgQueue.push_back(msg);
            hotkey.state.handled = true;
            continue;
        }
    }
}

// Translate inputs into messages using the active keymap(s)
void InputSystem::ProcessInputQueue(Depot &depot)
{
#if 0
    bool lCtrl = buttons[SDL_SCANCODE_LCTRL].Active(true);
    static bool ctrlWasPressed = 0;
    if (lCtrl && lCtrl != ctrlWasPressed) {
        printf("");
    }
    ctrlWasPressed = lCtrl;
#endif

    // Reset state
    for (int i = 0; i < FDOV_SCANCODE_COUNT; i++) {
        buttons[i].BeginFrame();
    }
    for (Keymap &keymap : depot.keymap) {
        for (KeymapHotkey &hotkey : keymap.hotkeys) {
            hotkey.state.BeginFrame();
        }
    }

    // Process each input event
    for (const InputEvent &e : inputQueue) {
        // Process a single event
        buttons[e.scancode].Set(e.down, depot.Now());

        if (CheckHotkeysDebug(depot, e)) {
            continue;
        }

        for (Keymap &keymap : depot.keymap) {
            // Check if the event triggered any new hotkeys
            CheckHotkeys(depot.Now(), buttons, keymap, depot.msgQueue);
        }
    }
    inputQueue.clear();

    for (Keymap &keymap : depot.keymap) {
        // Trigger commands for repeating hotkeys that are still active but
        // didn't change state (i.e. handled == false check in Active())
        for (KeymapHotkey &hotkey : keymap.hotkeys) {
            bool active = (hotkey.flags & Hotkey_Hold) && hotkey.state.Active();
            if (active) {
                Message msg{};
                msg.uid = keymap.uid;
                msg.type = hotkey.msgType;
                // TODO: Set msg.data if necessary? Hmm..
                depot.msgQueue.push_back(msg);
            }
        }
    }
}

