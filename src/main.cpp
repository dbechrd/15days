#include "common/message.h"
#include "facets/depot.h"
#include "systems/audio_system.h"
#include "systems/combat_system.h"
#include "systems/event_system_sdl.h"
#include "systems/input_system.h"
#include "systems/movement_system.h"
#include "systems/physics_system.h"
#include "systems/render_system.h"
#include "systems/sprite_system.h"
#include "systems/text_system.h"
#include "systems/trigger_system.h"
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"
#include <cassert>
#include <cstdio>

DLB_ASSERT_HANDLER(dlb_assert_callback) {
    printf("[%s:%u] %s\n", filename, line, expr);
}
dlb_assert_handler_def *dlb_assert_handler = dlb_assert_callback;

double clock_now(void)
{
    static uint64_t freq = SDL_GetPerformanceFrequency();
    uint64_t counter = SDL_GetPerformanceCounter();
    double now = (double)counter / freq;
    return now;
}

UID create_global_keymap(Depot &depot)
{
    // TODO: This is maybe "Menu" or something?
    UID uid = depot.Alloc();
    printf("%u: global keymap\n", uid);

    Keymap *keymap = (Keymap *)depot.AddFacet(uid, Facet_Keymap);
    keymap->hotkeys.emplace_back(FDOV_SCANCODE_QUIT, 0, 0, Hotkey_Press, MsgType_Input_Quit);
    keymap->hotkeys.emplace_back(SDL_SCANCODE_ESCAPE, 0, 0, Hotkey_Press, MsgType_Input_Quit);

    return uid;
}

UID create_player(Depot &depot)
{
    UID uid = depot.Alloc();
    printf("%u: player\n", uid);

    Keymap *keymap = (Keymap *)depot.AddFacet(uid, Facet_Keymap);
    keymap->hotkeys.emplace_back(FDOV_SCANCODE_MOUSE_LEFT, 0, 0, Hotkey_Press, MsgType_Input_Primary);
    keymap->hotkeys.emplace_back(FDOV_SCANCODE_MOUSE_RIGHT, 0, 0, Hotkey_Hold, MsgType_Input_Secondary);
    keymap->hotkeys.emplace_back(SDL_SCANCODE_W, 0, 0, Hotkey_Hold, MsgType_Input_Up);
    keymap->hotkeys.emplace_back(SDL_SCANCODE_A, 0, 0, Hotkey_Hold, MsgType_Input_Left);
    keymap->hotkeys.emplace_back(SDL_SCANCODE_S, 0, 0, Hotkey_Hold, MsgType_Input_Down);
    keymap->hotkeys.emplace_back(SDL_SCANCODE_D, 0, 0, Hotkey_Hold, MsgType_Input_Right);

    depot.AddFacet(uid, Facet_Combat);
    Sprite *sprite = (Sprite *)depot.AddFacet(uid, Facet_Sprite);
    SpriteSystem::InitSprite(*sprite);

    Position *position = (Position *)depot.AddFacet(uid, Facet_Position);
    position->pos = {
        SCREEN_W / 2.0f - sprite->size.x / 2.0f,
        SCREEN_H / 2.0f - sprite->size.y / 2.0f,
    };

    Body *body = (Body *)depot.AddFacet(uid, Facet_Body);
    //body->gravity = 0.0f;
    body->friction = 0.2f;

    return uid;
}

UID create_narrator(Depot &depot, UID subject, TTF_Font *font)
{
    UID uid = depot.Alloc();
    printf("%u: narrator\n", uid);

    Position *position = (Position *)depot.AddFacet(uid, Facet_Position);
    //int windowW = 0, windowH = 0;
    //SDL_GetWindowSize(renderSystem.window, &windowW, &windowH);
    //position->pos.x = windowW / 2.0f;
    //position->pos.y = 200.0f;
    position->pos.x = SCREEN_W / 2.0f;
    position->pos.y = 200;

    Text *text = (Text *)depot.AddFacet(uid, Facet_Text);
    text->font = font;
    text->text = "15 Days";
    text->color = C255(COLOR_WHITE);

    // TODO: NarratorSystem
    // - NarratorTrigger (UID, NarrationEvent_LeaveScreen)
    //   - Checks if position.pos + sprite.size outside of screen w/h
    // - NarratorSystem::Update();
    //   - Iterate all NarratorTrigger facets and check triggers
    //   - If any triggers fired, add Msg_NarratorSays to narratorQueue
    // - NarratorSystem::Draw(narratorQueue, drawList);
    //   - Generate draw commands from the message queue

    return uid;
}

void add_trigger(Depot &depot, UID subject, UID target)
{
    UID uidPrimary = depot.Alloc();
    printf("%u: trigger primary\n", uidPrimary);
    Trigger *triggerInputPrimary = (Trigger *)depot.AddFacet(uidPrimary, Facet_Trigger);
    triggerInputPrimary->trigger = MsgType_Input_Primary;
    triggerInputPrimary->message.uid = target;
    triggerInputPrimary->message.type = MsgType_Trigger_Text_Change;
    triggerInputPrimary->message.data.trigger_text_change.text = "Primary";
    triggerInputPrimary->message.data.trigger_text_change.color = C255(COLOR_RED);

    UID uidSecondary = depot.Alloc();
    printf("%u: trigger secondary\n", uidSecondary);
    Trigger *triggerInputSecondary = (Trigger *)depot.AddFacet(uidSecondary, Facet_Trigger);
    triggerInputSecondary->trigger = MsgType_Input_Secondary;
    triggerInputSecondary->message.uid = target;
    triggerInputSecondary->message.type = MsgType_Trigger_Text_Change;
    triggerInputSecondary->message.data.trigger_text_change.text = "Secondary";
    triggerInputSecondary->message.data.trigger_text_change.color = C255(COLOR_BLUE);

    TriggerList *triggerList = (TriggerList *)depot.AddFacet(subject, Facet_TriggerList);
    triggerList->triggers.push_back(uidPrimary);
    triggerList->triggers.push_back(uidSecondary);
}

//void *fdov_malloc_func(size_t size)
//{
//    void *ptr = malloc(size);
//    printf("malloc %p (%zu bytes)\n", ptr, size);
//    return ptr;
//}
//void *fdov_calloc_func(size_t nmemb, size_t size)
//{
//    void *ptr = calloc(nmemb, size);
//    printf("calloc %p (%zu bytes)\n", ptr, nmemb * size);
//    return ptr;
//}
//void *fdov_realloc_func(void *mem, size_t size)
//{
//    void *ptr = realloc(mem, size);
//    printf("realloc %p -> %p (%zu bytes)\n", mem, ptr, size);
//    return ptr;
//}
//void fdov_free_func(void *mem)
//{
//    free(mem);
//    printf("free %p\n", mem);
//}

int main(int argc, char *argv[])
{
    //vec4 cBlack  = {   0,   0,   0, 255 };
    //vec4 cWhite  = { 255, 255, 255, 255 };
    //vec4 cBeige  = { 224, 186, 139, 255 };
    //vec4 cPink   = { 255, 178, 223, 255 };
    //vec4 cPurple = {  55,  31,  69, 255 };
    //vec4 cGreen  = { 147, 255, 155, 255 };
    //vec4 cBlue   = { 130, 232, 255, 255 };
    //vec4 cYellow = { 255, 232, 150, 255 };
    //vec4 cOrange = { 255, 124,  30, 255 };

    int err;
    assert(FDOV_FIRST_SCANCODE == SDL_NUM_SCANCODES);

    for (int i = 0; i < argc; i++) {
        printf("argv[%d] = %s\n", i, argv[i]);
    }

    //SDL_SetMemoryFunctions(
    //    fdov_malloc_func,
    //    fdov_calloc_func,
    //    fdov_realloc_func,
    //    fdov_free_func
    //);

    // There aren't really the same kind of "system", maybe use a different word
    DepotSystem    depotSystem    {};
    EventSystemSDL eventSystemSDL {};
    InputSystem    inputSystem    {};

    // These are probably order-dependent
    MovementSystem movementSystem {};
    PhysicsSystem  physicsSystem  {};
    CombatSystem   combatSystem   {};
    SpriteSystem   spriteSystem   {};
    TriggerSystem  triggerSystem  {};  // might react to any gameplay system
    AudioSystem    audioSystem    {};  // might react to triggers
    TextSystem     textSystem     {};  // last because it might render debug UI for any system
    RenderSystem   renderSystem   {};

    err = renderSystem.Init("15days", SCREEN_W, SCREEN_H);
    if (err) return err;

    if (TTF_Init() < 0) {
        SDL_Log("Couldn't initialize TTF: %s\n", TTF_GetError());
        return -1;
    }

    err = audioSystem.Init();
    if (err) return err;

    // TODO: Resource loader (maybe RenderSystem::LoadFont()?)
    const char *fontFilename = "font/KarminaBold.otf";
    TTF_Font *font = TTF_OpenFont(fontFilename, 64);
    if (!font) {
        SDL_Log("Failed to load font %s: %s\n", fontFilename, SDL_GetError());
        return -1;
    }

    Depot &playDepot = depotSystem.ForState(GameState_Play);

    // Create an entity to hold the global keymap (the plan is to have a global
    // keymap per gamestate eventually)
    create_global_keymap(playDepot);

    // Create player/narrator
    UID player = create_player(playDepot);
    UID narrator = create_narrator(playDepot, player, font);
    add_trigger(playDepot, player, narrator);

    // Start the game
    depotSystem.TransitionTo(GameState_Play);

    // https://github.com/grimfang4/SDL_FontCache
    // https://github.com/libsdl-org/SDL_ttf/blob/main/showfont.c

    InputQueue inputQueue {};  // raw input (abstracted from platform)
    MsgQueue   msgQueue   {};  // messages generated by commands/systems

    uint64_t frame{};
    double now{};
    const double dt = 1.0 / 60.0;
    while (renderSystem.Running()) {
        frame++;

        // Time is money
        now = clock_now();

        // TODO: Frame arena
        inputQueue.clear();
        msgQueue.clear();

        // Reset text
        Message resetNarrator{};
        resetNarrator.uid = narrator;
        resetNarrator.type = MsgType_Trigger_Text_Change;
        resetNarrator.data.trigger_text_change.text = "Neutral";
        resetNarrator.data.trigger_text_change.color = C255(COLOR_GRAY_4);
        msgQueue.push_back(resetNarrator);

        // Update game state
        depotSystem.BeginFrame();
        Depot &depot = depotSystem.Current();

        // Collect SDL events into the appropriate queues
        eventSystemSDL.ProcessEvents(inputQueue, msgQueue);

        // TODO(dlb): Top-down stack order (e.g. menu handles input before game
        // when open, and marks all inputs as handled to prevent it from leaking
        // into the game, and a UI window like inventory does the same, but
        // only if the mouse is within the window bounds or smth)
        //
        // Translate inputs into messages using the active keymap(s)
        for (Keymap &keymap : depot.keymap) {
            inputSystem.ProcessInput(now, inputQueue, keymap, msgQueue);
        }

        // TODO: Should all messages in the queue be handled next frame?
        //       i.e. double-buffer the msgQueue to avoid order dependencies?
        // Forward commands to any system that might want to react to them
        movementSystem.React(now, depot, msgQueue);
        physicsSystem.React(now, depot, msgQueue);
        combatSystem.React(now, depot, msgQueue);
        spriteSystem.React(now, depot, msgQueue);
        triggerSystem.React(now, depot, msgQueue);
        audioSystem.React(now, depot, msgQueue);
        textSystem.React(now, depot, msgQueue);
        renderSystem.React(now, depot, msgQueue);

        // Update systems
        movementSystem.Behave(now, depot, dt);
        physicsSystem.Behave(now, depot, dt);
        combatSystem.Behave(now, depot, dt);
        spriteSystem.Behave(now, depot, dt);
        audioSystem.Behave(now, depot, dt);
        textSystem.Behave(now, depot, dt);
        renderSystem.Behave(now, depot, dt);

        // Populate draw queue(s)
        DrawQueue spriteQueue{};
        DrawQueue textQueue{};
        spriteSystem.Display(now, depot, spriteQueue);
        textSystem.Display(now, depot, textQueue);

#if 0
        if (inputQueue.size() || msgQueue.size()) {
            printf("Frame #%llu\n", frame);
            if (inputQueue.size()) {
                printf("  input   : ");
                for (const auto &input : inputQueue) {
                    printf(" %3d%s", input.scancode, input.down ? "v" : "^");
                }
                putchar('\n');
            }
            if (msgQueue.size()) {
                printf("  messages: ");
                for (const auto &msg : msgQueue) {
                    printf(" %3d", msg.type);
                }
                putchar('\n');
            }
            putchar('\n');
        }
#endif

        // Render draw queue(s)
        renderSystem.Clear(C255(COLOR_AQUA));
        renderSystem.Flush(spriteQueue);
        renderSystem.Flush(textQueue);
        renderSystem.Present();

        // If you disable v-sync, you'll want this to prevent global warming
        //SDL_Delay(1);
    }

    // Clean up (other systems might want to do this in the future)
    renderSystem.DestroyDepot(playDepot);
    renderSystem.Destroy();

    TTF_CloseFont(font);
    TTF_Quit();

    // SDL is currently reporting 1 unfreed alloc, but I haven't bothered to
    // try to find it yet.
    printf("SDL reported %d unfreed allocations\n", SDL_GetNumAllocations());
    //getchar();
    return 0;
}

#define DLB_MATH_IMPLEMENTATION
#include "dlb/dlb_math.h"