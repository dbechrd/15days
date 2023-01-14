#pragma once
#include "../common/basic.h"
#include "../common/input.h"
#include "../common/message.h"

struct EventSystemSDL {
    // TODO: This should take windowEventQueue, textInputQueue, etc. perhaps
    void ProcessEvents(InputQueue &inputQueue);
};