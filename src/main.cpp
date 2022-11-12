#include "facets/depot.h"
#include "systems/audio_system.h"
#include "systems/combat_system.h"
#include "systems/event_system_sdl.h"
#include "systems/input_system.h"
#include "systems/render_system.h"
#include "systems/sprite_system.h"
#include "SDL/SDL.h"
#include <cassert>
#include <cstdio>

double clock_now(void) {
    static uint64_t freq = SDL_GetPerformanceFrequency();
    uint64_t counter = SDL_GetPerformanceCounter();
    double now = (double)counter / freq;
    return now;
}

int main(int argc, char *argv[])
{
    int err;
    assert(FDOV_FIRST_SCANCODE == SDL_NUM_SCANCODES);

    for (int i = 0; i < argc; i++) {
        printf("argv[%d] = %s\n", i, argv[i]);
    }

    RenderSystem renderSystem{};
    err = renderSystem.Init("15days", 1600, 900);
    if (err) return err;

    AudioSystem audioSystem{};
    err = audioSystem.Init();
    if (err) return err;

    // Useful to get window width/height before toggling fullscreen
    //SDL_GetCurrentDisplayMode();

    DepotSystem    depotSystem    {};
    EventSystemSDL eventSystemSDL {};
    InputSystem    inputSystem    {};
    SpriteSystem   spriteSystem   {};
    CombatSystem   combatSystem   {};

    {
        depotSystem.Init(GameState_Play);
        Depot &depot = depotSystem.Current();

        UID player = depot.Alloc();

        Keymap *keymap = (Keymap *)depot.AddFacet(player, Facet_Keymap);
        keymap->hotkeys.emplace_back(SDL_SCANCODE_ESCAPE, 0, 0, Hotkey_Press, Command_QuitRequested);
        keymap->hotkeys.emplace_back(FDOV_SCANCODE_MOUSE_LEFT, 0, 0, Hotkey_Hold, Command_Primary);

        Position *position = (Position *)depot.AddFacet(player, Facet_Position);
        position->pos = { 100, 100 };

        depot.AddFacet(player, Facet_Combat);
        Sprite *sprite = (Sprite *)depot.AddFacet(player, Facet_Sprite);
        spriteSystem.InitSprite(*sprite);
    }

    // https://github.com/grimfang4/SDL_FontCache
    // https://github.com/libsdl-org/SDL_ttf/blob/main/showfont.c

    InputQueue inputQueue{};

    double now{};
    while (renderSystem.Running()) {
        now = clock_now();

        // TODO: Update GameState here based on requested GameState last frame
        // The gamestate can't change in the middle of the frame or the
        // InputSystem (and probably lots of other systems) will get confused.
        //g_gameState = GameState_Play;

        Depot &depot = depotSystem.Current();

        inputQueue.clear();
        eventSystemSDL.CollectEvents(inputQueue);

        for (Keymap &keymap : depot.keymap) {
            CommandQueue commandQueue{};

            // Translate events into commands, based on the given keymap
            inputSystem.TranslateEvents(now, inputQueue, keymap, commandQueue);

            // Do stuff with the commands
            combatSystem.ProcessCommands(now, depot, keymap.uid, commandQueue);

            // TODO: It probably doesn't make sense to have all commands go
            // straight to renderSystem. Something should process them, then
            // generate useful state for the renderer.
            renderSystem.ProcessCommands(now, commandQueue);
        }

        combatSystem.Update(now, depot);
        spriteSystem.Update(now, depot);

        DrawList drawList{};
        spriteSystem.Draw(now, depot, drawList);


        renderSystem.Clear();
        renderSystem.Render(drawList);
        renderSystem.Flip();

        //SDL_Delay(1);
    }

    renderSystem.Destroy();

    printf("SDL reported %d unfreed allocations\n", SDL_GetNumAllocations());
    return 0;
}
