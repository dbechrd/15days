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

void create_global_keymap(Depot &depot)
{
    // TODO: This is maybe "Menu" or something?
    UID uid = depot.Alloc();

    Keymap *keymap = (Keymap *)depot.AddFacet(uid, Facet_Keymap);
    keymap->hotkeys.emplace_back(FDOV_SCANCODE_QUIT, 0, 0, Hotkey_Press, MsgType_Input_Quit);
    keymap->hotkeys.emplace_back(SDL_SCANCODE_ESCAPE, 0, 0, Hotkey_Press, MsgType_Input_Quit);
}

void create_player(Depot &depot)
{
    UID player = depot.Alloc();

    Keymap *keymap = (Keymap *)depot.AddFacet(player, Facet_Keymap);
    keymap->hotkeys.emplace_back(FDOV_SCANCODE_MOUSE_LEFT, 0, 0, Hotkey_Press, MsgType_Input_Primary);
    keymap->hotkeys.emplace_back(FDOV_SCANCODE_MOUSE_RIGHT, 0, 0, Hotkey_Hold, MsgType_Input_Secondary);
    keymap->hotkeys.emplace_back(SDL_SCANCODE_W, 0, 0, Hotkey_Hold, MsgType_Input_Up);
    keymap->hotkeys.emplace_back(SDL_SCANCODE_A, 0, 0, Hotkey_Hold, MsgType_Input_Left);
    keymap->hotkeys.emplace_back(SDL_SCANCODE_S, 0, 0, Hotkey_Hold, MsgType_Input_Down);
    keymap->hotkeys.emplace_back(SDL_SCANCODE_D, 0, 0, Hotkey_Hold, MsgType_Input_Right);

    depot.AddFacet(player, Facet_Combat);
    Sprite *sprite = (Sprite *)depot.AddFacet(player, Facet_Sprite);
    SpriteSystem::InitSprite(*sprite);

    Position *position = (Position *)depot.AddFacet(player, Facet_Position);
    position->pos = {
        SCREEN_W / 2.0f - sprite->size.x / 2.0f,
        SCREEN_H / 2.0f - sprite->size.y / 2.0f,
    };

    Body *body = (Body *)depot.AddFacet(player, Facet_Body);
    //body->gravity = 0.0f;
    body->friction = 0.2f;
}

void create_narrator(Depot &depot, UID subject)
{
    UID narrator = depot.Alloc();

    Trigger *trigger = (Trigger *)depot.AddFacet(narrator, Facet_Trigger);
    trigger->subject = subject;
    trigger->target = narrator;
    trigger->type = Trigger_Collide;
    //trigger->command =
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

    RenderSystem renderSystem{};
    err = renderSystem.Init("15days", SCREEN_W, SCREEN_H);
    if (err) return err;

    if (TTF_Init() < 0) {
        SDL_Log("Couldn't initialize TTF: %s\n", TTF_GetError());
        return -1;
    }

    const char *fontFilename = "font/KarminaBold.otf";
    TTF_Font *font = TTF_OpenFont(fontFilename, 64);
    if (!font) {
        SDL_Log("Failed to load font %s: %s\n", fontFilename, SDL_GetError());
        return -1;
    }

    // TTF_STYLE_NORMAL        0x00
    // TTF_STYLE_BOLD          0x01
    // TTF_STYLE_ITALIC        0x02
    // TTF_STYLE_UNDERLINE     0x04
    // TTF_STYLE_STRIKETHROUGH 0x08
    //TTF_SetFontStyle(font, TTF_STYLE_NORMAL);

    vec4 cBlack  = {   0,   0,   0, 255 };
    vec4 cWhite  = { 255, 255, 255, 255 };
    vec4 cBeige  = { 224, 186, 139, 255 };
    vec4 cPink   = { 255, 178, 223, 255 };
    vec4 cPurple = {  55,  31,  69, 255 };
    vec4 cGreen  = { 147, 255, 155, 255 };
    vec4 cBlue   = { 130, 232, 255, 255 };
    vec4 cYellow = { 255, 232, 150, 255 };
    vec4 cOrange = { 255, 124,  30, 255 };

    SDL_Texture *textTex{};
    SDL_Rect textRect{};
    {
        const char *fontStr = "15 Days";
        SDL_Surface *text = TTF_RenderText_Blended(font, fontStr, { 255, 255, 255, 255 });
        textTex = SDL_CreateTextureFromSurface(renderSystem.renderer, text);
        SDL_FreeSurface(text);

        int windowW = 0, windowH = 0;
        SDL_GetWindowSize(renderSystem.window, &windowW, &windowH);

        textRect = {
            (windowW - text->w) / 2,
            200, //(windowH - text->h) / 2,
            text->w,
            text->h
        };
    }

    AudioSystem audioSystem{};
    err = audioSystem.Init();
    if (err) return err;

    // These aren't currently order-dependent, but might be one day
    CombatSystem   combatSystem   {};
    DepotSystem    depotSystem    {};
    EventSystemSDL eventSystemSDL {};
    InputSystem    inputSystem    {};
    MovementSystem movementSystem {};
    PhysicsSystem  physicsSystem  {};
    SpriteSystem   spriteSystem   {};

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

    InputQueue inputQueue {};  // raw input (abstracted from platform)
    MsgQueue   msgQueue   {};  // messages generated by commands/systems

    uint64_t frame{};
    double now{};
    while (renderSystem.Running()) {
        frame++;

        // Time is money
        now = clock_now();

        // TODO: Frame arena
        inputQueue.clear();
        msgQueue.clear();

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

        // Forward commands to any system that might want to react to them
        // TODO: Should all messages in the queue be handled next frame?
        //       i.e. double-buffer the msgQueue to avoid order dependencies?
        movementSystem.ProcessMessages(now, depot, msgQueue);
        combatSystem.ProcessMessages(now, depot, msgQueue);
        renderSystem.ProcessMessages(now, depot, msgQueue);

        // TODO: NarratorSystem
        // - NarratorTrigger (UID, NarrationEvent_LeaveScreen)
        //   - Checks if position.pos + sprite.size outside of screen w/h
        // - NarratorSystem::Update();
        //   - Iterate all NarratorTrigger facets and check triggers
        //   - If any triggers fired, add Msg_NarratorSays to narratorQueue
        // - NarratorSystem::Draw(narratorQueue, drawList);
        //   - Generate draw commands from the message queue

        // Update systems
        physicsSystem.Update(now, depot);
        combatSystem.Update(now, depot);
        spriteSystem.Update(now, depot);

        // Populate draw list(s)
        DrawList drawList{};
        spriteSystem.Draw(now, depot, drawList);

        // Render draw list(s)
        renderSystem.Clear(C255(COLOR_AQUA));
        renderSystem.Render(drawList);

        static int colorIdx = 1;
        //static double colorLastChange = 0;
        //if (now - colorLastChange > 0.5) {
        //    colorIdx = (colorIdx + 1) % SDL_arraysize(colors);
        //    colorLastChange = now;
        //}

        SDL_SetTextureColorMod(textTex, 255, 255, 255);
        SDL_RenderCopy(renderSystem.renderer, textTex, NULL, &textRect);

#if 1
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

        renderSystem.Flip();

        // If you disable v-sync, you'll want this to prevent global warming
        //SDL_Delay(1);
    }

    SDL_DestroyTexture(textTex);
    TTF_CloseFont(font);
    TTF_Quit();

    // Clean up (other systems might want to do this in the future)
    renderSystem.Destroy();

    // SDL is currently reporting 1 unfreed alloc, but I haven't bothered to
    // try to find it yet.
    printf("SDL reported %d unfreed allocations\n", SDL_GetNumAllocations());
    //getchar();
    return 0;
}

#define DLB_MATH_IMPLEMENTATION
#include "dlb/dlb_math.h"