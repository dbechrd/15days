#include "depot.h"
#include "SDL/SDL.h"
#include <cassert>
#include <cstdio>

double clock_now(void) {
    static uint64_t freq = SDL_GetPerformanceFrequency();
    uint64_t counter = SDL_GetPerformanceCounter();
    double now = (double)counter / freq;
    return now;
}

struct InputEvent {
    int scancode{};
    bool down{};    // true for KEYDOWN, false for KEYUP
};
struct InputSystem {
private:
    std::vector<InputEvent> inputQueue{};

public:
    std::vector<CommandType> commandQueue{};

    void BeginFrame(Depot &depot, GameState gameState)
    {
        inputQueue.clear();
        commandQueue.clear();

        InputButtons &inputButtons = depot.inputButtons.back();
        for (int i = 0; i < InputButton_Count; i++) {
            inputButtons.buttons[i].BeginFrame();
        }

        InputKeymap &inputKeymap = depot.inputKeymap.back();
        for (InputHotkey &hotkey : inputKeymap.hotkeys[gameState]) {
            hotkey.state.BeginFrame();
        }
    }

    void Enqueue(int scancode, bool isDown)
    {
        inputQueue.push_back({ scancode, isDown });
    }

    void Update(Depot &depot, double now, GameState gameState)
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
            bool active = (hotkey.flags & HotkeyTriggerFlag_Active) && hotkey.state.Active();
            if (active) {
                commandQueue.push_back(hotkey.command);
            }
        }
    }

private:
    void CheckHotkeys(Depot &depot, double now, GameState gameState)
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

                bool triggered = (hotkey.flags & HotkeyTriggerFlag_Trigger) && hotkey.state.Triggered();
                bool released = (hotkey.flags & HotkeyTriggerFlag_Release) && hotkey.state.Released();
                if (triggered || released) {
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
} g_inputSystem{};

int main(int argc, char *argv[])
{
    for (int i = 0; i < argc; i++) {
        printf("argv[%d] = %s\n", i, argv[i]);
    }

    int sdl_init = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    if (sdl_init < 0) {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "15days",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1600,
        900,
        SDL_WINDOW_OPENGL
    );
    if (!window) {
        printf("Failed to create window: %s\n", SDL_GetError());
        return -1;
    }

#if 0
    int audioDrivers = SDL_GetNumAudioDrivers();
    for (int i = 0; i < audioDrivers; i++) {
        printf("audio_driver[%d] = %s\n", i, SDL_GetAudioDriver(i));
    }

    int audioDevices = SDL_GetNumAudioDevices(0);
    for (int i = 0; i < audioDevices; i++) {
        SDL_AudioSpec spec{};
        if (SDL_GetAudioDeviceSpec(i, 0, &spec) != 0) {
            printf("Failed to query audio_device[%d] spec: %s\n", i, SDL_GetError());
            continue;
        }

        printf("audio_device[%d]:\n"
            "  freq           : %d\n"
            "  channels       : %u\n"
            "  buffer samples : %d (per channel)\n"
            "  buffer size    : %u bytes\n",
            i,
            spec.freq,
            spec.channels,
            spec.samples,
            spec.size
        );
    }

    int videoDrivers = SDL_GetNumVideoDrivers();
    for (int i = 0; i < videoDrivers; i++) {
        printf("video_driver[%d] = %s\n", i, SDL_GetVideoDriver(i));
    }

    int drivers = SDL_GetNumRenderDrivers();
    for (int i = 0; i < drivers; i++) {
        SDL_RendererInfo info{};
        if (SDL_GetRenderDriverInfo(i, &info) < 0) {
            printf("Failed to query render_driver[%d] info: %s\n", i, SDL_GetError());
            continue;
        }

        printf("render_driver[%d]:\n"
            "  name                : %s\n"
            "  num_texture_formats : %u\n"
            "  max_texture_width   : %d\n"
            "  max_texture_height  : %d\n",
            i,
            info.name,
            info.num_texture_formats,
            info.max_texture_width,
            info.max_texture_height
        );
    }
#endif

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        return -1;
    }

    printf("Audio driver: %s\n", SDL_GetCurrentAudioDriver());
    printf("Video driver: %s\n", SDL_GetCurrentVideoDriver());

    // Useful to get window width/height before toggling fullscreen
    //SDL_GetCurrentDisplayMode();

    Depot depot{};
    depot.inputButtons.push_back({});
    depot.inputKeymap.push_back({});

    InputKeymap &keymap = depot.inputKeymap.back();
    keymap.hotkeys[GameState_Play].emplace_back(
        SDL_SCANCODE_ESCAPE, 0, 0, HotkeyTriggerFlag_Trigger, Command_QuitRequested);
    keymap.hotkeys[GameState_Play].emplace_back(
        FDOV_SCANCODE_MOUSE_LEFT, 0, 0, HotkeyTriggerFlag_Active, Command_Primary);

    double now{};
    SDL_Event evt{};

    bool quit = false;
    while (!quit) {
        now = clock_now();

        // TODO: Update GameState here based on requested GameState last frame
        // The gamestate can't change in the middle of the frame or the
        // InputSystem (and probably lots of other systems) will get confused.

        g_inputSystem.BeginFrame(depot, GameState_Play);

        while (SDL_PollEvent(&evt)) {
            switch (evt.type) {
                case SDL_QUIT: {
                    g_inputSystem.Enqueue(FDOV_SCANCODE_QUIT, true);
                    break;
                } case SDL_KEYDOWN: {
                    g_inputSystem.Enqueue(evt.key.keysym.scancode, true);
                    break;
                } case SDL_KEYUP: {
                    g_inputSystem.Enqueue(evt.key.keysym.scancode, false);
                    break;
                } case SDL_MOUSEBUTTONDOWN: case SDL_MOUSEBUTTONUP: {
                    int scancode = 0;
                    switch (evt.button.button) {
                        case SDL_BUTTON_LEFT   : scancode = FDOV_SCANCODE_MOUSE_LEFT;   break;
                        case SDL_BUTTON_MIDDLE : scancode = FDOV_SCANCODE_MOUSE_MIDDLE; break;
                        case SDL_BUTTON_RIGHT  : scancode = FDOV_SCANCODE_MOUSE_RIGHT;  break;
                        case SDL_BUTTON_X1     : scancode = FDOV_SCANCODE_MOUSE_X1;     break;
                        case SDL_BUTTON_X2     : scancode = FDOV_SCANCODE_MOUSE_X1;     break;
                    }
                    if (scancode) {
                        if (evt.button.state == SDL_PRESSED) {
                            g_inputSystem.Enqueue(scancode, true);
                        } else if (evt.button.state == SDL_RELEASED) {
                            g_inputSystem.Enqueue(scancode, false);
                        }
                    }
                    break;
                }
            }
        }

        g_inputSystem.Update(depot, now, GameState_Play);

        SDL_Color drawColor { 15, 50, 70, SDL_ALPHA_OPAQUE };
        for (CommandType commandType : g_inputSystem.commandQueue) {
            switch (commandType) {
                case Command_QuitRequested: {
                    quit = true;
                    break;
                }
                case Command_Primary: {
                    drawColor = { 150, 70, 70, SDL_ALPHA_OPAQUE };
                    break;
                }
                default: {
                    break;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, drawColor.r, drawColor.g, drawColor.b, drawColor.a);
        SDL_RenderFillRect(renderer, 0);

        SDL_RenderPresent(renderer);
        SDL_Delay(10);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    printf("SDL reported %d unfreed allocations\n", SDL_GetNumAllocations());
    return 0;
}
