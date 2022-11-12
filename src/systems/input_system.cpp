#include "input_system.h"
#include <cassert>

void InputSystem::TranslateEvents(
    double now,
    const InputQueue &inputQueue,
    Keymap &keymap,
    CommandQueue &commandQueue)
{
    // Reset state
    for (int i = 0; i < FDOV_SCANCODE_COUNT; i++) {
        buttons[i].BeginFrame();
    }
    for (KeymapHotkey &hotkey : keymap.hotkeys) {
        hotkey.state.BeginFrame();
    }

    // Process each input event
    for (const InputEvent &e : inputQueue) {
        // Process a single event
        buttons[e.scancode].Set(e.down, now);

        // Check if the event triggered any new hotkeys
        CheckHotkeys(now, buttons, keymap, commandQueue);
    }

    // Trigger commands for repeating hotkeys that are still active but
    // didn't change state (i.e. handled == false check in Active())
    for (KeymapHotkey &hotkey : keymap.hotkeys) {
        bool active = (hotkey.flags & Hotkey_Hold) && hotkey.state.Active();
        if (active) {
            commandQueue.push_back(hotkey.command);
        }
    }
}

void InputSystem::CheckHotkeys(
    double now,
    ButtonState buttons[FDOV_SCANCODE_COUNT],
    Keymap &keymap,
    CommandQueue &commandQueue)
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

        bool a = buttons[k0].Active();
        bool b = !k1 || buttons[k1].Active();
        bool c = !k2 || buttons[k2].Active();
        bool active = a && b && c;

        hotkey.state.Set(active, now);

        bool pressed = (hotkey.flags & Hotkey_Press) && hotkey.state.Pressed();
        bool released = (hotkey.flags & Hotkey_Release) && hotkey.state.Released();
        if (pressed || released) {
            // NOTE: It's fine if these set buttons[0].handled to true
            buttons[k0].handled = true;
            buttons[k1].handled = true;
            buttons[k2].handled = true;
            commandQueue.push_back(hotkey.command);
            hotkey.state.handled = true;
            continue;
        }
    }
}
