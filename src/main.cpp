#include "common/message.h"
#include "facets/depot.h"
#include "systems/audio_system.h"
#include "systems/card_system.h"
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
    keymap->hotkeys.emplace_back(HotkeyMod_None, FDOV_SCANCODE_QUIT, 0, 0, Hotkey_Press, MsgType_Render_Quit);
    keymap->hotkeys.emplace_back(HotkeyMod_None, SDL_SCANCODE_ESCAPE, 0, 0, Hotkey_Press, MsgType_Render_Quit);
    keymap->hotkeys.emplace_back(HotkeyMod_None, SDL_SCANCODE_V, 0, 0, Hotkey_Press, MsgType_Render_ToggleVsync);

    return uid;
}

UID create_player(Depot &depot, AudioSystem &audioSystem, TTF_Font *font)
{
    UID uidPlayer = depot.Alloc();
    printf("%u: player\n", uidPlayer);

    Cursor *cursor = (Cursor *)depot.AddFacet(uidPlayer, Facet_Cursor);
    UNUSED(cursor);
    // TODO: Assign specific input pointer device to this player?

    // TODO: Make keymaps be per-mode *not* Depots. There should only be 1 depot.
    // TODO: Make a "card drag" mode that has a different keymap for the player

    Keymap *keymap = (Keymap *)depot.AddFacet(uidPlayer, Facet_Keymap);

    keymap->hotkeys.emplace_back(HotkeyMod_Ctrl, FDOV_SCANCODE_MOUSE_LEFT, 0, 0, Hotkey_Press, MsgType_Card_DragBegin);
    keymap->hotkeys.emplace_back(HotkeyMod_Ctrl, FDOV_SCANCODE_MOUSE_LEFT, 0, 0, Hotkey_Release | Hotkey_Handled, MsgType_Card_DragEnd);

    keymap->hotkeys.emplace_back(HotkeyMod_Any, FDOV_SCANCODE_MOUSE_LEFT, 0, 0, Hotkey_Press, MsgType_Combat_Primary);
    keymap->hotkeys.emplace_back(HotkeyMod_Any, FDOV_SCANCODE_MOUSE_RIGHT, 0, 0, Hotkey_Hold, MsgType_Combat_Secondary);
    //keymap->hotkeys.emplace_back(HotkeyMod_Any, FDOV_SCANCODE_MOUSE_RIGHT, 0, 0, Hotkey_Press | Hotkey_Handled, MsgType_Combat_Secondary_Press);

    keymap->hotkeys.emplace_back(HotkeyMod_Shift, SDL_SCANCODE_W, 0, 0, Hotkey_Hold, MsgType_Movement_RunUp);
    keymap->hotkeys.emplace_back(HotkeyMod_Shift, SDL_SCANCODE_A, 0, 0, Hotkey_Hold, MsgType_Movement_RunLeft);
    keymap->hotkeys.emplace_back(HotkeyMod_Shift, SDL_SCANCODE_S, 0, 0, Hotkey_Hold, MsgType_Movement_RunDown);
    keymap->hotkeys.emplace_back(HotkeyMod_Shift, SDL_SCANCODE_D, 0, 0, Hotkey_Hold, MsgType_Movement_RunRight);
    keymap->hotkeys.emplace_back(HotkeyMod_None, SDL_SCANCODE_W, 0, 0, Hotkey_Hold, MsgType_Movement_WalkUp);
    keymap->hotkeys.emplace_back(HotkeyMod_None, SDL_SCANCODE_A, 0, 0, Hotkey_Hold, MsgType_Movement_WalkLeft);
    keymap->hotkeys.emplace_back(HotkeyMod_None, SDL_SCANCODE_S, 0, 0, Hotkey_Hold, MsgType_Movement_WalkDown);
    keymap->hotkeys.emplace_back(HotkeyMod_None, SDL_SCANCODE_D, 0, 0, Hotkey_Hold, MsgType_Movement_WalkRight);
    keymap->hotkeys.emplace_back(HotkeyMod_Any, SDL_SCANCODE_SPACE, 0, 0, Hotkey_Press, MsgType_Movement_Jump);

    depot.AddFacet(uidPlayer, Facet_Combat);
    Sprite *sprite = (Sprite *)depot.AddFacet(uidPlayer, Facet_Sprite);
    SpriteSystem::InitSprite(*sprite);

    Position *position = (Position *)depot.AddFacet(uidPlayer, Facet_Position);
    position->pos = {
        SCREEN_W / 2.0f - sprite->size.x / 2.0f,
        SCREEN_H / 2.0f - sprite->size.y / 2.0f,
    };

    Body *body = (Body *)depot.AddFacet(uidPlayer, Facet_Body);
    body->gravity = -50.0f;
    body->friction = 0.001f;
    //body->drag = 0.001f;
    body->drag = 0.05f;
    body->restitution = 0.0f;
    body->jumpImpulse = 800.0f;
    body->speed = 20.0f;
    body->runMult = 2.0f;
    float mass = 1.0f;
    body->invMass = 1.0f / mass;

    Text *debugText = (Text *)depot.AddFacet(uidPlayer, Facet_Text);
    debugText->font = font;
    debugText->str = 0;
    debugText->align = TextAlign_VBottom_HCenter;
    debugText->color = C255(COLOR_WHITE);

    UID uidSoundPrimary = depot.Alloc();
    Sound *soundPrimary = (Sound *)depot.AddFacet(uidSoundPrimary, Facet_Sound);
    audioSystem.InitSound(*soundPrimary, "audio/primary.wav");

    UID uidSoundSecondary = depot.Alloc();
    Sound *soundSecondary = (Sound *)depot.AddFacet(uidSoundSecondary, Facet_Sound);
    audioSystem.InitSound(*soundSecondary, "audio/secondary.wav");

    UID uidTriggerPrimary = depot.Alloc();
    printf("%u: player trigger primary sound\n", uidTriggerPrimary);
    Trigger *triggerInputPrimary = (Trigger *)depot.AddFacet(uidTriggerPrimary, Facet_Trigger);
    triggerInputPrimary->trigger = MsgType_Combat_Primary;
    triggerInputPrimary->message.uid = uidSoundPrimary;
    triggerInputPrimary->message.type = MsgType_Audio_PlaySound;
    triggerInputPrimary->message.data.audio_playsound.override = true;

    UID uidTriggerSecondary = depot.Alloc();
    printf("%u: player trigger secondary sound\n", uidTriggerSecondary);
    Trigger *triggerInputSecondary = (Trigger *)depot.AddFacet(uidTriggerSecondary, Facet_Trigger);
    triggerInputSecondary->trigger = MsgType_Combat_Secondary;
    triggerInputSecondary->message.uid = uidSoundSecondary;
    triggerInputSecondary->message.type = MsgType_Audio_PlaySound;

    TriggerList *triggerList = (TriggerList *)depot.AddFacet(uidPlayer, Facet_TriggerList);
    triggerList->triggers.push_back(uidTriggerPrimary);
    triggerList->triggers.push_back(uidTriggerSecondary);

    return uidPlayer;
}

UID create_narrator(Depot &depot, UID subject, TTF_Font *font)
{
    UID uidNarrator = depot.Alloc();
    printf("%u: narrator\n", uidNarrator);

    Position *position = (Position *)depot.AddFacet(uidNarrator, Facet_Position);
    //int windowW = 0, windowH = 0;
    //SDL_GetWindowSize(renderSystem.window, &windowW, &windowH);
    //position->pos.x = windowW / 2.0f;
    //position->pos.y = 200.0f;
    position->pos.x = SCREEN_W / 2.0f;
    position->pos.y = 200;

    Text *text = (Text *)depot.AddFacet(uidNarrator, Facet_Text);
    text->font = font;
    text->str = "15 Days";
    text->align = TextAlign_VBottom_HCenter;
    text->color = C255(COLOR_WHITE);

    UID uidTriggerPrimary = depot.Alloc();
    printf("%u: narrator trigger primary text change\n", uidTriggerPrimary);
    Trigger *triggerInputPrimary = (Trigger *)depot.AddFacet(uidTriggerPrimary, Facet_Trigger);
    triggerInputPrimary->trigger = MsgType_Combat_Primary;
    triggerInputPrimary->message.uid = uidNarrator;
    triggerInputPrimary->message.type = MsgType_Text_UpdateText;
    triggerInputPrimary->message.data.text_updatetext.str = "Primary";
    triggerInputPrimary->message.data.text_updatetext.color = C255(COLOR_RED);

    UID uidTriggerSecondary = depot.Alloc();
    printf("%u: narrator trigger secondary text change\n", uidTriggerSecondary);
    Trigger *triggerInputSecondary = (Trigger *)depot.AddFacet(uidTriggerSecondary, Facet_Trigger);
    triggerInputSecondary->trigger = MsgType_Combat_Secondary;
    triggerInputSecondary->message.uid = uidNarrator;
    triggerInputSecondary->message.type = MsgType_Text_UpdateText;
    triggerInputSecondary->message.data.text_updatetext.str = "Secondary";
    triggerInputSecondary->message.data.text_updatetext.color = C255(COLOR_DODGER);

    TriggerList *triggerList = (TriggerList *)depot.GetFacet(subject, Facet_TriggerList);
    triggerList->triggers.push_back(uidTriggerPrimary);
    triggerList->triggers.push_back(uidTriggerSecondary);

    // TODO: NarratorSystem
    // - NarratorTrigger (UID, NarrationEvent_LeaveScreen)
    //   - Checks if position.pos + sprite.size outside of screen w/h
    // - NarratorSystem::Update();
    //   - Iterate all NarratorTrigger facets and check triggers
    //   - If any triggers fired, add Msg_NarratorSays to narratorQueue
    // - NarratorSystem::Draw(narratorQueue, drawList);
    //   - Generate draw commands from the message queue

    return uidNarrator;
}

UID create_fps_counter(Depot &depot, TTF_Font *font)
{
    UID uidFpsCounter = depot.Alloc();
    printf("%u: fps counter\n", uidFpsCounter);

    Position *position = (Position *)depot.AddFacet(uidFpsCounter, Facet_Position);
    //int windowW = 0, windowH = 0;
    //SDL_GetWindowSize(renderSystem.window, &windowW, &windowH);
    //position->pos.x = windowW / 2.0f;
    //position->pos.y = 200.0f;
    position->pos.x = 10;
    position->pos.y = 10;

    Text *text = (Text *)depot.AddFacet(uidFpsCounter, Facet_Text);
    text->font = font;
    text->str = "00 fps (00.00 ms)";
    text->align = TextAlign_VTop_HLeft;
    text->color = C255(COLOR_WHITE);

    // TODO: NarratorSystem
    // - NarratorTrigger (UID, NarrationEvent_LeaveScreen)
    //   - Checks if position.pos + sprite.size outside of screen w/h
    // - NarratorSystem::Update();
    //   - Iterate all NarratorTrigger facets and check triggers
    //   - If any triggers fired, add Msg_NarratorSays to narratorQueue
    // - NarratorSystem::Draw(narratorQueue, drawList);
    //   - Generate draw commands from the message queue

    return uidFpsCounter;
}

UID create_card(Depot &depot, AudioSystem &audioSystem, TTF_Font *font)
{
    UID uidCard = depot.Alloc();
    printf("%u: card\n", uidCard);

    Sprite *sprite = (Sprite *)depot.AddFacet(uidCard, Facet_Sprite);
    SpriteSystem::InitSprite(*sprite);

    Position *position = (Position *)depot.AddFacet(uidCard, Facet_Position);
    position->pos = { 100, 100 };

    Body *body = (Body *)depot.AddFacet(uidCard, Facet_Body);
    body->gravity = -50.0f;
    body->friction = 0.001f;
    //body->drag = 0.001f;
    body->drag = 0.05f;
    body->restitution = 0.0f;
    body->jumpImpulse = 800.0f;
    body->speed = 20.0f;
    body->runMult = 2.0f;
    float mass = 1.0f;
    body->invMass = 1.0f / mass;

    Text *debugText = (Text *)depot.AddFacet(uidCard, Facet_Text);
    debugText->font = font;
    debugText->str = 0;
    debugText->align = TextAlign_VBottom_HCenter;
    debugText->color = C255(COLOR_WHITE);

    UID uidSoundDragBegin = depot.Alloc();
    Sound *soundDragBegin = (Sound *)depot.AddFacet(uidSoundDragBegin, Facet_Sound);
    audioSystem.InitSound(*soundDragBegin, "audio/primary.wav");

    UID uidSoundDragEnd = depot.Alloc();
    Sound *soundDragEnd = (Sound *)depot.AddFacet(uidSoundDragEnd, Facet_Sound);
    audioSystem.InitSound(*soundDragEnd, "audio/secondary.wav");

    UID uidTriggerDragBegin = depot.Alloc();
    printf("%u: card trigger drag begin sound\n", uidTriggerDragBegin);
    Trigger *triggerInputDragBegin = (Trigger *)depot.AddFacet(uidTriggerDragBegin, Facet_Trigger);
    triggerInputDragBegin->trigger = MsgType_Card_DragBegin;
    triggerInputDragBegin->message.uid = uidSoundDragBegin;
    triggerInputDragBegin->message.type = MsgType_Audio_PlaySound;
    triggerInputDragBegin->message.data.audio_playsound.override = true;

    UID uidTriggerDragEnd = depot.Alloc();
    printf("%u: card trigger drag end sound\n", uidTriggerDragEnd);
    Trigger *triggerInputDragEnd = (Trigger *)depot.AddFacet(uidTriggerDragEnd, Facet_Trigger);
    triggerInputDragEnd->trigger = MsgType_Card_DragEnd;
    triggerInputDragEnd->message.uid = uidSoundDragEnd;
    triggerInputDragEnd->message.type = MsgType_Audio_PlaySound;
    triggerInputDragEnd->message.data.audio_playsound.override = true;

    TriggerList *triggerList = (TriggerList *)depot.AddFacet(uidCard, Facet_TriggerList);
    triggerList->triggers.push_back(uidTriggerDragBegin);
    triggerList->triggers.push_back(uidTriggerDragEnd);

    return uidCard;
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
    CardSystem     cardSystem     {};
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
    const char *fontFancyFilename = "font/KarminaBold.otf";
    TTF_Font *fontFancy = TTF_OpenFont(fontFancyFilename, 64);
    if (!fontFancy) {
        SDL_Log("Failed to load font %s: %s\n", fontFancyFilename, SDL_GetError());
        return -1;
    }
    const char *fontFixedFilename = "font/Hack-Bold.ttf";
    TTF_Font *fontFixed = TTF_OpenFont(fontFixedFilename, 16);
    if (!fontFixed) {
        SDL_Log("Failed to load font %s: %s\n", fontFixedFilename, SDL_GetError());
        return -1;
    }

    depotSystem.Init(GameState_Play);
    Depot &playDepot = depotSystem.ForState(GameState_Play);

    // Create an entity to hold the global keymap (the plan is to have a global
    // keymap per gamestate eventually)
    create_global_keymap(playDepot);

    // Create player/narrator
    UID player = create_player(playDepot, audioSystem, fontFixed);
    UID narrator = create_narrator(playDepot, player, fontFancy);
    UID fpsCounter = create_fps_counter(playDepot, fontFixed);
    UID card = create_card(playDepot, audioSystem, fontFixed);
    UNUSED(card);

    // Start the game
    depotSystem.TransitionTo(GameState_Play);

    // https://github.com/grimfang4/SDL_FontCache
    // https://github.com/libsdl-org/SDL_ttf/blob/main/showfont.c

    InputQueue inputQueue {};  // raw input (abstracted from platform)

    uint64_t frame{};
    double physicsAccum{};
    double nowPrev{};
    double now{};
    const double fixedDt = 1.0 / 60.0;
    double realDtSmooth = fixedDt;
    while (renderSystem.Running()) {
        frame++;

        // Time is money
        nowPrev = now;
        now = clock_now();
        double realDt = now - nowPrev;
        realDt = MIN(realDt, fixedDt);  // cap to prevent spiral while debugging
        realDtSmooth = LERP(realDtSmooth, realDt, 0.1);

        physicsAccum += realDt;
        int physicsIters = 0;
        while (physicsAccum >= fixedDt) {
            physicsIters++;
            physicsAccum -= fixedDt;
        }

        // Update game state
        depotSystem.BeginFrame();
        Depot &depot = depotSystem.Current();

        // Update FPS counter text
        size_t fpsCounterMaxLen = 32;
        char *fpsCounterBuf = (char *)depot.frameArena.Alloc(fpsCounterMaxLen);
        if (fpsCounterBuf) {
            snprintf(fpsCounterBuf, fpsCounterMaxLen, "%.2f fps (%.2f ms)", 1.0f / realDtSmooth, realDtSmooth * 1000.0f);

            Message updateFpsCounter{};
            updateFpsCounter.uid = fpsCounter;
            updateFpsCounter.type = MsgType_Text_UpdateText;
            updateFpsCounter.data.text_updatetext.str = fpsCounterBuf;
            updateFpsCounter.data.text_updatetext.color = C255(COLOR_WHITE);
            depot.msgQueue.push_back(updateFpsCounter);
        } else {
            printf("WARN: Failed to allocate enough frame arena space for fps counter string\n");
        }

        // Reset narrator text
        Message resetNarrator{};
        resetNarrator.uid = narrator;
        resetNarrator.type = MsgType_Text_UpdateText;
        resetNarrator.data.text_updatetext.str = "Neutral";
        resetNarrator.data.text_updatetext.color = C255(COLOR_GRAY_4);
        depot.msgQueue.push_back(resetNarrator);

        {
            // Collect SDL events into the appropriate queues
            eventSystemSDL.ProcessEvents(inputQueue, depot.msgQueue);

            // TODO(dlb): Top-down stack order (e.g. menu handles input before game
            // when open, and marks all inputs as handled to prevent it from leaking
            // into the game, and a UI window like inventory does the same, but
            // only if the mouse is within the window bounds or smth)
            //
            // Translate inputs into messages using the active keymap(s)
            for (Keymap &keymap : depot.keymap) {
                inputSystem.ProcessInput(now, inputQueue, keymap, depot.msgQueue);
            }
            inputQueue.clear();
        }

        // Forward commands to any system that might want to react to them
        // NOTE: Maybe this should be double-buffered with 1 frame delay?
        cardSystem.React(now, depot);
        movementSystem.React(now, depot);
        physicsSystem.React(now, depot);
        combatSystem.React(now, depot);
        spriteSystem.React(now, depot);
        triggerSystem.React(now, depot);
        audioSystem.React(now, depot);
        renderSystem.React(now, depot);

        // Update systems
        for (int i = 0; i < physicsIters; i++) {
            cardSystem.Behave(now, depot, fixedDt);
            movementSystem.Behave(now, depot, fixedDt);
            physicsSystem.Behave(now, depot, fixedDt);
            combatSystem.Behave(now, depot, fixedDt);
            spriteSystem.Behave(now, depot, fixedDt);
            audioSystem.Behave(now, depot, fixedDt);
            renderSystem.Behave(now, depot, fixedDt);
        }

        // Debug text may need to be updated, so update textSystem last
        textSystem.React(now, depot);
        for (int i = 0; i < physicsIters; i++) {
            textSystem.Behave(now, depot, fixedDt);
        }

        // Populate draw queue(s)
        DrawQueue spriteQueue{};
        DrawQueue textQueue{};
        spriteSystem.Display(now, depot, spriteQueue);
        textSystem.Display(now, depot, textQueue);

#if 0
        if (inputQueue.size() || depot.msgQueue.size()) {
            printf("Frame #%llu\n", frame);
            if (inputQueue.size()) {
                printf("  input   : ");
                for (const auto &input : inputQueue) {
                    printf(" %3d%s", input.scancode, input.down ? "v" : "^");
                }
                putchar('\n');
            }
            if (depot.msgQueue.size()) {
                printf("  messages: ");
                for (const auto &msg : depot.msgQueue) {
                    printf(" %3d", msg.type);
                }
                putchar('\n');
            }
            putchar('\n');
        }
#endif

        // Render draw queue(s)
        renderSystem.Clear(C_GRASS);
        renderSystem.Flush(playDepot, spriteQueue);
        renderSystem.Flush(playDepot, textQueue);
        renderSystem.Present();
        depotSystem.EndFrame();

        // If you disable v-sync, you'll want this to prevent global warming
        //SDL_Delay(1);
    }

    // Clean up (other systems might want to do this in the future)
    renderSystem.DestroyDepot(playDepot);
    audioSystem.DestroyDepot(playDepot);
    renderSystem.Destroy();
    audioSystem.Destroy();
    depotSystem.Destroy();

    TTF_CloseFont(fontFancy);
    TTF_CloseFont(fontFixed);
    TTF_Quit();

    // SDL is currently reporting 1 unfreed alloc, but I haven't bothered to
    // try to find it yet.
    printf("SDL reported %d unfreed allocations\n", SDL_GetNumAllocations());
    //getchar();
    return 0;
}

#define DLB_MATH_IMPLEMENTATION
#include "dlb/dlb_math.h"
