#include "event_system_sdl.h"
#include "input_system.h"
#include "../facets/depot.h"

void EventSystemSDL::ProcessEvents(Depot &depot)
{
    SDL_Event evt{};

    // TODO: Handle things like window resize event by generating a message
    // instead of pushing InputEvent into input queue.

#if 0
    SDL_Keymod mod = SDL_GetModState();
    inputQueue.push_back({ FDOV_SCANCODE_SHIFT, (bool)(mod & KMOD_SHIFT) });
    inputQueue.push_back({ FDOV_SCANCODE_CTRL, (bool)(mod & KMOD_CTRL) });
    inputQueue.push_back({ FDOV_SCANCODE_ALT, (bool)(mod & KMOD_ALT) });
#endif

    while (SDL_PollEvent(&evt)) {
        switch (evt.type) {
            case SDL_QUIT:
            {
                Message msgQuit{};
                msgQuit.uid = 0;
                msgQuit.type = MsgType_Render_Quit;
                depot.msgQueue.push_back(msgQuit);
                break;
            }
            case SDL_KEYDOWN:
            {
                if (!evt.key.repeat) {
                    depot.inputSystem.PushInputEvent(evt.key.keysym.scancode, true);
                }
                break;
            }
            case SDL_KEYUP:
            {
                if (!evt.key.repeat) {
                    depot.inputSystem.PushInputEvent(evt.key.keysym.scancode, false);
                }
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            {
                int scancode = 0;

                // Map mouse button to custom scancode
                switch (evt.button.button)
                {
                    case SDL_BUTTON_LEFT   : scancode = FDOV_SCANCODE_MOUSE_LEFT;   break;
                    case SDL_BUTTON_MIDDLE : scancode = FDOV_SCANCODE_MOUSE_MIDDLE; break;
                    case SDL_BUTTON_RIGHT  : scancode = FDOV_SCANCODE_MOUSE_RIGHT;  break;
                    case SDL_BUTTON_X1     : scancode = FDOV_SCANCODE_MOUSE_X1;     break;
                    case SDL_BUTTON_X2     : scancode = FDOV_SCANCODE_MOUSE_X1;     break;
                }

                // If we mapped it successfully, queue it
                if (scancode)
                {
                    if (evt.button.state == SDL_PRESSED) {
                        depot.inputSystem.PushInputEvent(scancode, true);
                    } else if (evt.button.state == SDL_RELEASED) {
                        depot.inputSystem.PushInputEvent(scancode, false);
                    }
                }
                break;
            }
            case SDL_MOUSEMOTION:
            {
                // whenever mouse moves, store x/y position somewhere
                // - OR -
                // whenever i need mouse position, request it from SDL
                //    compare to previously queried position, if different, generate mouse move trigger
            }
            default: break;
        }
    }
}