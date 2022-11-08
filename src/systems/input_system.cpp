#include "input_system.h"

InputSystem g_inputSystem{};

void InputSystem::BeginFrame(Depot &depot, GameState gameState)
{
    inputQueue.clear();
    commandQueue.clear();

    InputButtons &buttons = depot.inputButtons.back();
    InputKeymap &keymap = depot.inputKeymap.back();

    for (int i = 0; i < InputButton_Count; i++) {
        buttons.buttons[i].BeginFrame();
    }
    for (InputHotkey &hotkey : keymap.hotkeys[gameState]) {
        hotkey.state.BeginFrame();
    }
}

void InputSystem::Enqueue(int scancode, bool isDown)
{
    inputQueue.push_back({ scancode, isDown });
}

void InputSystem::Update(Depot &depot, double now, GameState gameState)
{
    // TODO: Maybe get gameState from a facet as well?? E.g. InputMode
    InputButtons &buttons = depot.inputButtons.back();
    InputKeymap &keymap = depot.inputKeymap.back();

    for (InputEvent &e : inputQueue) {
        // Process a single event
        buttons.buttons[e.scancode].Set(e.down, now);

        // Check if the event triggered any new hotkeys
        CheckHotkeys(buttons, keymap, now, gameState);
    }

    // Trigger commands for repeating hotkeys that are still active but
    // didn't change state (i.e. handled == false check in Active())
    for (InputHotkey &hotkey : keymap.hotkeys[gameState]) {
        bool active = (hotkey.flags & Hotkey_Hold) && hotkey.state.Active();
        if (active) {
            commandQueue.push_back(hotkey.command);
        }
    }
}

void InputSystem::CheckHotkeys(InputButtons &buttons, InputKeymap &keymap, double now, GameState gameState)
{
    // Determine if that event caused any hotkeys to trigger.
    // If so, queue a command.
    for (InputHotkey &hotkey : keymap.hotkeys[gameState]) {
        int k0 = hotkey.keys[0];
        int k1 = hotkey.keys[1];
        int k2 = hotkey.keys[2];

        // Ignore unbound hotkeys
        if (!k0) {
            assert(!k1);
            assert(!k2);
            continue;
        }

        bool a = buttons.buttons[k0].Active();
        bool b = !k1 || buttons.buttons[k1].Active();
        bool c = !k2 || buttons.buttons[k2].Active();
        bool active = a && b && c;

        hotkey.state.Set(active, now);

        bool pressed = (hotkey.flags & Hotkey_Press) && hotkey.state.Pressed();
        bool released = (hotkey.flags & Hotkey_Release) && hotkey.state.Released();
        if (pressed || released) {
            // NOTE: It's fine if these set buttons[0].handled to true
            buttons.buttons[k0].handled = true;
            buttons.buttons[k1].handled = true;
            buttons.buttons[k2].handled = true;
            commandQueue.push_back(hotkey.command);
            hotkey.state.handled = true;
            continue;
        }
    }
}
