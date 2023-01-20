#include "common/basic.h"
#include "common/message.h"
#include "facets/depot.h"

DLB_ASSERT_HANDLER(dlb_assert_callback) {
    printf("[%s:%u] %s\n", filename, line, expr);
}
dlb_assert_handler_def *dlb_assert_handler = dlb_assert_callback;

Error load_resource_db(Depot &depot, const char *filename, void **bufToDelete)
{
    size_t size{};
    void *data = SDL_LoadFile(filename, &size);
    if (!data) {
        SDL_LogError(0, "Failed to load db: %s\n", filename);
        return E_IO_ERROR;
    }

    const ResourceDB::Root *db = ResourceDB::GetRoot(data);
    flatbuffers::Verifier fbv((const u8 *)data, size);
    if (!ResourceDB::VerifyRootBuffer(fbv)) {
        SDL_LogError(0, "Verify failed: %s\n", filename);
        return E_VERIFY_FAILED;
    }

    depot.resources = db;

    // Preload spritesheet textures (to prevent lazy-loading later)
    for (const auto &sheet : *db->spritesheets()) {
        depot.renderSystem.FindOrCreateTextureBMP(depot, sheet->texture_key()->c_str());
    }

#if 1
    // DEBUG: Print spritesheet info
    printf("resdb %s\n", db->name()->c_str());
    for (const auto &sheet : *db->spritesheets()) {
        printf("  sheet %s has %d %dx%d cells \n",
            sheet->name()->c_str(),
            sheet->cell_count(),
            sheet->cell_width(),
            sheet->cell_height()
        );

        for (const auto &dbAnim : *sheet->animations()) {
            printf("    anim %s start %d count %d\n",
                dbAnim->name()->c_str(),
                dbAnim->frame_start(),
                dbAnim->frame_count()
            );
        }
    }
#endif

    // Credit: whaatsuuup in twitch chat noticed this wasn't here. If you forget
    // return values in functions meant to return things, all sorts of nonsensical
    // bullshit occurs.
    return E_SUCCESS;
}

UID create_global_keymap(Depot &depot)
{
    // TODO: This is maybe "Menu" or something?
    UID uid = depot.Alloc("global_keymap");

    Keymap *keymap = (Keymap *)depot.AddFacet(uid, Facet_Keymap);
    keymap->hotkeys.emplace_back(HotkeyMod_None, SDL_SCANCODE_ESCAPE, 0, 0, Hotkey_Press, MsgType_Render_Quit);
    keymap->hotkeys.emplace_back(HotkeyMod_None, SDL_SCANCODE_V, 0, 0, Hotkey_Press, MsgType_Render_ToggleVsync);
    keymap->hotkeys.emplace_back(HotkeyMod_None, SDL_SCANCODE_F11, 0, 0, Hotkey_Press, MsgType_Render_ToggleFullscreen);
    keymap->hotkeys.emplace_back(HotkeyMod_None, SDL_SCANCODE_F, 0, 0, Hotkey_Press, MsgType_Render_DbgSetFontNext);
    keymap->hotkeys.emplace_back(HotkeyMod_Shift, SDL_SCANCODE_F, 0, 0, Hotkey_Press, MsgType_Render_DbgSetFontPrev);

    return uid;
}

UID create_cursor(Depot &depot)
{
    UID uidCursor = depot.Alloc("cursor");

    Cursor *cursor = (Cursor *)depot.AddFacet(uidCursor, Facet_Cursor);
    cursor->quickClickMaxDt = 0.1;

    Position *position = (Position *)depot.AddFacet(uidCursor, Facet_Position);
    float x = 0;
    float y = 0;
    SDL_GetMouseState(&x, &y);
    position->pos = { x, y };
    position->size = { 1, 1 };

#if 0
    Text *text = (Text *)depot.AddFacet(uidCursor, Facet_Text);
    text->fontKey = "opensans_bold_16";
    text->str = "00 fps (00.00 ms)";
    text->align = TextAlign_VTop_HLeft;
    text->offset = { 16, 16 };
#endif

    //Keymap *keymap = (Keymap *)depot.AddFacet(uidCursor, Facet_Keymap);
    //keymap->hotkeys.emplace_back(HotkeyMod_None, FDOV_SCANCODE_MOUSE_LEFT, 0, 0, Hotkey_Press, MsgType_Cursor_PrimaryPress);
    //keymap->hotkeys.emplace_back(HotkeyMod_None, FDOV_SCANCODE_MOUSE_LEFT, 0, 0, Hotkey_Release | Hotkey_Handled, MsgType_Cursor_PrimaryRelease);

    //depot.triggerSystem.Trigger_Audio_PlaySound(depot, uidCursor, MsgType_Cursor_Notify_DragBegin, "drag_begin");
    //depot.triggerSystem.Trigger_Audio_PlaySound(depot, uidCursor, MsgType_Cursor_Notify_DragEnd, "drag_end");

    return uidCursor;
}


void fps_update_text(Depot &depot, const Message &msg, const Trigger &trigger, void *userData)
{
    switch (msg.type) {
        case MsgType_Render_FrameBegin:
        {
            const double fps = depot.FpsSmooth();
            const double dt = depot.DtSmooth();
            const double dtMillis = dt * 1000.0f;
            const double realDt = depot.RealDt();
            const double realDtMillis = realDt * 1000.0f;

            const size_t fpsCounterMaxLen = 2048;
            char *fpsCounterBuf = (char *)depot.frameArena.Alloc(fpsCounterMaxLen);
            if (fpsCounterBuf) {
                snprintf(fpsCounterBuf, fpsCounterMaxLen,
                    "%.2f fps (sim: %.2f ms, real: %.2f ms, hashes: %zu)\n"
#if FDOV_SHOW_TODO_LIST
                    "ConfigFile.fbb (for keybinds, window pos/size, etc.)\n"
                    "DefaultSaveFile.fbb (for new games)\n"
                    "make deck disappear when empty\n"
                    //"font atlas / glyph cache\n"
                    //"text drop shadow\n"
                    "runes\n"
                    "volume control\n"
                    "sell stuff\n"
                    "buy stuffffffff\n"
                    "roll random attribs\n"
                    "card groups (inventory?)\n"
                    "click location cards to teleport there\n"
                    "charges\n"
                    "cards that recharge other cards\n"
                    "cards with timers (e.g. bomb)\n"
                    "networking (?)\n"
#endif
                    ,
                    fps, dtMillis, realDtMillis, depot.Hashes()
                );

                depot.textSystem.PushUpdateText(depot, trigger.message.uid,
                    { 0.0f, 20.0f }, C255(COLOR_WHITE), fpsCounterBuf);
            } else {
                printf("WARN: Failed to allocate enough frame arena space for fps counter string\n");
            }
            break;
        }
        default: break;
    }
}

UID create_fps_counter(Depot &depot)
{
    UID uidFpsCounter = depot.Alloc("fps_counter");

    FpsCounter *fpsCounter = (FpsCounter *)depot.AddFacet(uidFpsCounter, Facet_FpsCounter);
    UNUSED(fpsCounter);

    Position *position = (Position *)depot.AddFacet(uidFpsCounter, Facet_Position);
    //int windowW = 0, windowH = 0;
    //SDL_GetWindowSize(renderSystem.window, &windowW, &windowH);
    //position->pos.x = windowW / 2.0f;
    //position->pos.y = 200.0f;
    position->pos.x = 10;
    position->pos.y = 90;

    Text *text = (Text *)depot.AddFacet(uidFpsCounter, Facet_Text);
    text->fontKey = "opensans_bold_16";
    //text->font = depot.textSystem.LoadFont(depot, "font/pricedown_bl.ttf", 16);

    text->str = "00 fps (00.00 ms)";
    text->align = TextAlign_VTop_HLeft;

    Histogram *histo = (Histogram *)depot.AddFacet(uidFpsCounter, Facet_Histogram);
    for (int i = 1; i <= 100; i++) {
        histo->values.push_back(i);
    };

    depot.triggerSystem.Trigger_Special_RelayAllMessages(depot, uidFpsCounter, uidFpsCounter, fps_update_text);

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

bool combat_try_attack(Depot &depot, Combat *combat)
{
    bool canAttack = combat->Idle();
    if (canAttack) {
        combat->attackStartedAt = depot.Now();
        combat->attackCooldown = 0.2;
    }
    return canAttack;
}
bool combat_try_defend(Depot &depot, Combat *combat)
{
    bool canDefend = combat->Idle() || combat->Defending();
    if (canDefend) {
        combat->defendStartedAt = depot.Now();
        combat->defendCooldown = 0.6;
    }
    return canDefend;
}
bool combat_try_idle(Depot &depot, Combat *combat)
{
    bool idleBegin = false;
    if (combat->attackStartedAt) {
        DLB_ASSERT(combat->attackCooldown);
        float attackAlpha = (depot.Now() - combat->attackStartedAt) / combat->attackCooldown;
        if (attackAlpha >= 1.0) {
            combat->attackStartedAt = 0;
            combat->attackCooldown = 0;
            idleBegin = true;
        }
    }
    if (combat->defendStartedAt) {
        DLB_ASSERT(combat->defendCooldown);
        float defendAlpha = (depot.Now() - combat->defendStartedAt) / combat->defendCooldown;
        if (defendAlpha >= 1.0) {
            combat->defendStartedAt = 0;
            combat->defendCooldown = 0;
            idleBegin = true;
        }
    }
    return idleBegin;
}

void player_callback(Depot &depot, const Message &msg, const Trigger &trigger, void *userData)
{
    Combat *combat = (Combat *)depot.GetFacet(trigger.message.uid, Facet_Combat);
    if (!combat) {
        return;
    }

    switch (msg.type) {
        case MsgType_Render_FrameBegin:
        {
            if (combat_try_idle(depot, combat)) {
                depot.textSystem.PushUpdateNarrator(depot, {}, C255(COLOR_GRAY_4), "`wNeutral", 5, true);
            }
            break;
        }
        case MsgType_Combat_Primary:
        {
            if (combat_try_attack(depot, combat)) {
                depot.audioSystem.PushPlaySound(depot, "sfx_player_attack");
                depot.textSystem.PushUpdateNarrator(depot, {}, C255(COLOR_GRAY_4), "`rPrimary", 5, true);
            }
            break;
        }
        case MsgType_Combat_Secondary:
        {
            if (combat_try_defend(depot, combat)) {
                depot.audioSystem.PushPlaySound(depot, "sfx_player_defend");
                depot.textSystem.PushUpdateNarrator(depot, {}, C255(COLOR_GRAY_4), "`cSecondary", 5, true);
            }
            break;
        }
        default: break;
    }
}

UID create_player(Depot &depot)
{
    // TODO: Make a "save_file_default" that is loaded when the player first
    // begins a new game, which creates entities like the player, default cards,
    // or whatever.
    const char *sheetKey = "sheet_player";
    const ResourceDB::Spritesheet *sheet = depot.resources->spritesheets()->LookupByKey(sheetKey);
    if (!sheet) {
        SDL_LogError(0, "Couldn't find spritesheet for player %s\n", sheetKey);
        return 0;
    }

    UID uidPlayer = depot.Alloc("player");

    Position *position = (Position *)depot.AddFacet(uidPlayer, Facet_Position);
    position->size.x = sheet->cell_width();
    position->size.y = sheet->cell_height();
    position->pos = {
        SCREEN_W / 2.0f - position->size.x / 2.0f,
        SCREEN_H / 2.0f - position->size.y / 2.0f,
    };

    depot.AddFacet(uidPlayer, Facet_Combat);
    Sprite *sprite = (Sprite *)depot.AddFacet(uidPlayer, Facet_Sprite);
    depot.spriteSystem.InitSprite(depot, *sprite, C255(COLOR_WHEAT), sheetKey, "anim_player");

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
    debugText->fontKey = "opensans_bold_16";
    debugText->str = 0;
    debugText->align = TextAlign_VBottom_HCenter;

    {
        // TODO: Make keymaps be per-mode *not* Depots. There should only be 1 depot.
        // TODO: Make a "card drag" mode that has a different keymap for the player

        Keymap *keymap = (Keymap *)depot.AddFacet(uidPlayer, Facet_Keymap);

        keymap->hotkeys.emplace_back(HotkeyMod_Any, SDL_SCANCODE_LEFT, 0, 0, Hotkey_Press, MsgType_Combat_Primary);
        keymap->hotkeys.emplace_back(HotkeyMod_Any, SDL_SCANCODE_RIGHT, 0, 0, Hotkey_Hold, MsgType_Combat_Secondary);
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
    }

    depot.triggerSystem.Trigger_Special_RelayAllMessages(depot, uidPlayer, uidPlayer, player_callback);

    return uidPlayer;
}

void campfire_callback(Depot &depot, const Message &msg, const Trigger &trigger, void *userData)
{
    UID uidCampfire = msg.uid;

    switch (msg.type) {
        case MsgType_Material_StateChange: {
            const ResourceDB::MaterialStates &newStates = msg.data.material_statechange.newStates;
            const ResourceDB::MaterialStates &deltaStates = msg.data.material_statechange.deltaStates;

            if (deltaStates & ResourceDB::MaterialStates_OnFire) {
                if (newStates & ResourceDB::MaterialStates_OnFire) {
                    depot.audioSystem.PushPlaySound(depot, "sfx_fire_start", true);
                    depot.audioSystem.PushStopSound(depot, "sfx_fire_extinguish");
                    depot.spriteSystem.PushUpdateAnimation(uidCampfire, "sheet_campfire_small", "anim_burning");
                } else {
                    depot.audioSystem.PushPlaySound(depot, "sfx_fire_extinguish", true);
                    depot.audioSystem.PushStopSound(depot, "sfx_fire_start");
                    depot.spriteSystem.PushUpdateAnimation(uidCampfire, "sheet_campfire_small", "anim_unlit");
                }
            }
            break;
        }
        default: break;
    }
}

void create_cards(Depot &depot)
{
    // TODO: Make a "save_file_default" that is loaded when the player first
    // begins a new game, which creates entities like the player, default cards,
    // or whatever.

    // Decks
    depot.cardSystem.PushSpawnDeck(depot, "card_proto_deck", { 600, 300, 0 }, 0, 25);
    depot.cardSystem.PushSpawnDeck(depot, "card_proto_deck", { 700, 300, 0 }, 0, 25);

    // Cards
    //depot.cardSystem.PushSpawnCard(depot, "card_proto_lighter", { 700, 300, 0 });
    //depot.cardSystem.PushSpawnCard(depot, "card_proto_water_bucket", { 800, 300, 0 });
    //depot.cardSystem.PushSpawnCard(depot, "card_proto_bomb", { 900, 300, 0 });
    depot.cardSystem.PushSpawnCard(depot, "card_proto_camp", { 900, 300, 0 }, 0);
    depot.cardSystem.PushSpawnCard(depot, "card_proto_campfire", { 200, 500, 0 }, campfire_callback);
}

void say_hello(Depot &depot)
{
    depot.textSystem.PushUpdateNarrator(depot, {}, C255(COLOR_WHITE), " ", 2);
    depot.textSystem.PushUpdateNarrator(depot, {}, C255(COLOR_WHITE), "Hello! `k...", 1);
    depot.textSystem.PushUpdateNarrator(depot, {}, C255(COLOR_WHITE), "Hello! `k..", 1);
    depot.textSystem.PushUpdateNarrator(depot, {}, C255(COLOR_WHITE), "Hello! `k.", 1);
    depot.textSystem.PushUpdateNarrator(depot, {}, C255(COLOR_WHITE), "Hello!", 1);
    depot.textSystem.PushUpdateNarrator(depot, {}, C255(COLOR_WHITE), "Welcome to 15 days! `k...", 1);
    depot.textSystem.PushUpdateNarrator(depot, {}, C255(COLOR_WHITE), "Welcome to 15 days! `k..", 1);
    depot.textSystem.PushUpdateNarrator(depot, {}, C255(COLOR_WHITE), "Welcome to 15 days! `k.", 1);
    depot.textSystem.PushUpdateNarrator(depot, {}, C255(COLOR_WHITE), "Welcome to 15 days!", 1);
    depot.textSystem.PushUpdateNarrator(depot, {}, C255(COLOR_WHITE), "This is a game about cards. `k...", 1);
    depot.textSystem.PushUpdateNarrator(depot, {}, C255(COLOR_WHITE), "This is a game about cards. `k..", 1);
    depot.textSystem.PushUpdateNarrator(depot, {}, C255(COLOR_WHITE), "This is a game about cards. `k.", 1);
    depot.textSystem.PushUpdateNarrator(depot, {}, C255(COLOR_WHITE), "This is a game about cards.", 1);
    depot.textSystem.PushUpdateNarrator(depot, {}, C255(COLOR_WHITE), "Click them and see what happens! `k....", 1);
    depot.textSystem.PushUpdateNarrator(depot, {}, C255(COLOR_WHITE), "Click them and see what happens! `k...", 1);
    depot.textSystem.PushUpdateNarrator(depot, {}, C255(COLOR_WHITE), "Click them and see what happens! `k..", 1);
    depot.textSystem.PushUpdateNarrator(depot, {}, C255(COLOR_WHITE), "Click them and see what happens! `k.", 1);
    depot.textSystem.PushUpdateNarrator(depot, {}, C255(COLOR_WHITE), "Click them and see what happens!", 1);
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
    Error err = E_SUCCESS;
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

    Depot *depotPtr = new Depot();
    Depot &depot = *depotPtr;
    err = depot.Init(GameState_Play);
    if (!err) {
        dlb_rand32_seed(SDL_GetTicks());

        // TODO: Move this into depot init?
        void *bufToDelete = 0;
        Error resourceDbErr = load_resource_db(depot, "db/ResourceDB.fbb", &bufToDelete);
        if (!resourceDbErr) {
            create_global_keymap(depot);
            create_cursor(depot);
            create_fps_counter(depot);
            create_player(depot);
            create_cards(depot);
            say_hello(depot);

            // Run the game
            depot.TransitionTo(GameState_Play);
            depot.Run();
        } else {
            SDL_LogError(0, "Uh oh, resdb failed to load");
            DLB_ASSERT(!"Uh oh, resdb failed to load");
            err = E_INIT_FAILED;
        }

        depot.Destroy();
        delete depotPtr;

        TTF_Quit();

        // TODO: Move this into depot destroy?
        SDL_free(bufToDelete);

        // SDL is currently reporting 1 unfreed alloc, but I haven't bothered to
        // try to find it yet.
        printf("SDL reported %d unfreed allocations\n", SDL_GetNumAllocations());
        //getchar();
    }

    return err;
}

#define DLB_MATH_IMPLEMENTATION
#include "dlb/dlb_math.h"
#define DLB_RAND_IMPLEMENTATION
#include "dlb/dlb_rand.h"
#define DLB_HASH_IMPLEMENTATION
#include "dlb/dlb_hash.h"
#define DLB_MURMUR3_IMPLEMENTATION
#include "dlb/dlb_murmur3.h"