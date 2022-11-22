#pragma once
#include "../common/input.h"
#include "../common/message.h"
#include <vector>

struct EventSystemSDL {
    // TODO: This should take windowEventQueue, textInputQueue, etc. perhaps
    void ProcessEvents(InputQueue &inputQueue, MsgQueue &msgQueue);
};