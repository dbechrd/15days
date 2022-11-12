#include "event_system_sdl.h"
#include "input_system.h"
#include "SDL/SDL.h"

void EventSystemSDL::CollectEvents(std::vector<InputEvent> &inputQueue)
{
    SDL_Event evt{};

    // Redirect SDL events to their appropriate subsystem
    while (SDL_PollEvent(&evt)) {
        switch (evt.type) {
            case SDL_QUIT:
            {
                inputQueue.push_back({ FDOV_SCANCODE_QUIT, true });
                break;
            }
            case SDL_KEYDOWN:
            {
                inputQueue.push_back({ evt.key.keysym.scancode, true });
                break;
            }
            case SDL_KEYUP:
            {
                inputQueue.push_back({ evt.key.keysym.scancode, false });
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
                        inputQueue.push_back({ scancode, true });
                    } else if (evt.button.state == SDL_RELEASED) {
                        inputQueue.push_back({ scancode, false });
                    }
                }
                break;
            }
            default: break;
        }
    }
}