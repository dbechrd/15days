#pragma once
#include <vector>

struct InputEvent;

struct EventSystemSDL {
    // TODO: This should take windowEventQueue, textInputQueue, etc. perhaps
    void CollectEvents(std::vector<InputEvent> &inputQueue);
};