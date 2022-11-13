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

double clock_now(void)
{
    static uint64_t freq = SDL_GetPerformanceFrequency();
    uint64_t counter = SDL_GetPerformanceCounter();
    double now = (double)counter / freq;
    return now;
}

void create_global_keymap(Depot &depot)
{
    UID uid = depot.Alloc();

    Keymap *keymap = (Keymap *)depot.AddFacet(uid, Facet_Keymap);
    keymap->hotkeys.emplace_back(FDOV_SCANCODE_QUIT, 0, 0, Hotkey_Press, Command_QuitRequested);
    keymap->hotkeys.emplace_back(SDL_SCANCODE_ESCAPE, 0, 0, Hotkey_Press, Command_QuitRequested);
}

void create_player(Depot &depot)
{
    UID player = depot.Alloc();

    Keymap *keymap = (Keymap *)depot.AddFacet(player, Facet_Keymap);
    keymap->hotkeys.emplace_back(FDOV_SCANCODE_MOUSE_LEFT, 0, 0, Hotkey_Press, Command_Primary);
    keymap->hotkeys.emplace_back(FDOV_SCANCODE_MOUSE_RIGHT, 0, 0, Hotkey_Hold, Command_Secondary);

    Position *position = (Position *)depot.AddFacet(player, Facet_Position);
    position->pos = { 100, 100 };

    depot.AddFacet(player, Facet_Combat);
    Sprite *sprite = (Sprite *)depot.AddFacet(player, Facet_Sprite);
    SpriteSystem::InitSprite(*sprite);
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

    // These aren't currently order-dependent, but might be one day
    DepotSystem    depotSystem    {};
    EventSystemSDL eventSystemSDL {};
    InputSystem    inputSystem    {};
    SpriteSystem   spriteSystem   {};
    CombatSystem   combatSystem   {};

    Depot &playDepot = depotSystem.ForState(GameState_Play);

    // Create an entity to hold the global keymap (the plan is to have a global
    // keymap per gamestate eventually)
    create_global_keymap(playDepot);

    // Create a "player"
    create_player(playDepot);

    // Start the game
    depotSystem.TransitionTo(GameState_Play);

    // https://github.com/grimfang4/SDL_FontCache
    // https://github.com/libsdl-org/SDL_ttf/blob/main/showfont.c

    double now{};
    while (renderSystem.Running()) {
        // Time is money
        now = clock_now();

        // Update game state
        depotSystem.BeginFrame();
        Depot &depot = depotSystem.Current();

        // Collect SDL events into the appropriate queues
        InputQueue inputQueue{};
        eventSystemSDL.CollectEvents(inputQueue);

        // Generate and process game commands
        for (Keymap &keymap : depot.keymap) {
            // Translate events into commands, based on the active keymap(s)
            CommandQueue commandQueue{};
            inputSystem.TranslateEvents(now, inputQueue, keymap, commandQueue);

            // Forward commands to any system that might want to react to them
            combatSystem.ProcessCommands(now, depot, keymap.uid, commandQueue);
            renderSystem.ProcessCommands(now, commandQueue);
        }

        // Update systems
        combatSystem.Update(now, depot);
        spriteSystem.Update(now, depot);

        // Populate draw list(s)
        DrawList drawList{};
        spriteSystem.Draw(now, depot, drawList);

        // Render draw list(s)
        renderSystem.Clear();
        renderSystem.Render(drawList);
        renderSystem.Flip();

        // If you disable v-sync, you'll want this to prevent global warming
        //SDL_Delay(1);
    }

    // Clean up (other systems might want to do this in the future)
    renderSystem.Destroy();

    // SDL is currently reporting 1 unfreed alloc, but I haven't bothered to
    // try to find it yet.
    printf("SDL reported %d unfreed allocations\n", SDL_GetNumAllocations());
    return 0;
}
