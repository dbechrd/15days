#include "input_system.h"

InputSystem g_inputSystem{};

void InputSystem::BeginFrame(Depot &depot, GameState gameState)
{
    inputQueue.clear();
    commandQueue.clear();

    InputButtons &inputButtons = depot.inputButtons.back();
    InputKeymap &inputKeymap = depot.inputKeymap.back();

    for (int i = 0; i < InputButton_Count; i++) {
        inputButtons.buttons[i].BeginFrame();
    }
    for (InputHotkey &hotkey : inputKeymap.hotkeys[gameState]) {
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
    InputButtons &inputButtons = depot.inputButtons.back();
    InputKeymap &inputKeymap = depot.inputKeymap.back();

    for (InputEvent &e : inputQueue) {
        // Process a single event
        inputButtons.buttons[e.scancode].Set(e.down, now);

        // Check if the event triggered any new hotkeys
        CheckHotkeys(depot, now, gameState);
    }

    // Trigger commands for repeating hotkeys that are still active but
    // didn't change state (i.e. handled == false check in Active())
    for (InputHotkey &hotkey : inputKeymap.hotkeys[gameState]) {
        bool active = (hotkey.flags & Hotkey_Hold) && hotkey.state.Active();
        if (active) {
            commandQueue.push_back(hotkey.command);
        }
    }
}

void InputSystem::CheckHotkeys(Depot &depot, double now, GameState gameState)
{
    // TODO: Maybe get gameState from a facet as well?? E.g. InputMode
    InputButtons &inputButtons = depot.inputButtons.back();
    InputKeymap &inputKeymap = depot.inputKeymap.back();

    // Determine if that event caused any hotkeys to trigger.
    // If so, queue a command.
    for (InputHotkey &hotkey : inputKeymap.hotkeys[gameState]) {
        int k0 = hotkey.keys[0];
        int k1 = hotkey.keys[1];
        int k2 = hotkey.keys[2];

        if (k0) {
            bool a = inputButtons.buttons[k0].Active();
            bool b = !k1 || inputButtons.buttons[k1].Active();
            bool c = !k2 || inputButtons.buttons[k2].Active();
            bool active = a && b && c;

            hotkey.state.Set(active, now);

            bool pressed = (hotkey.flags & Hotkey_Press) && hotkey.state.Pressed();
            bool released = (hotkey.flags & Hotkey_Release) && hotkey.state.Released();
            if (pressed || released) {
                // NOTE: It's fine if these set buttons[0].handled to true
                inputButtons.buttons[k0].handled = true;
                inputButtons.buttons[k1].handled = true;
                inputButtons.buttons[k2].handled = true;
                commandQueue.push_back(hotkey.command);
                hotkey.state.handled = true;
                continue;
            }
        } else {
            // Ignore unbound hotkeys
            assert(!k1);
            assert(!k2);
        }
    }
}
