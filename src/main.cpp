#include "common/basic.h"
#include "common/message.h"
#include "facets/depot.h"
#include "fbg/resourcedb_generated.h"

DLB_ASSERT_HANDLER(dlb_assert_callback) {
    printf("[%s:%u] %s\n", filename, line, expr);
}
dlb_assert_handler_def *dlb_assert_handler = dlb_assert_callback;

const char *debugFont = "font/OpenSans-Bold.ttf";

UID create_global_keymap(Depot &depot)
{
    // TODO: This is maybe "Menu" or something?
    UID uid = depot.Alloc("global_keymap");

    Keymap *keymap = (Keymap *)depot.AddFacet(uid, Facet_Keymap);
    keymap->hotkeys.emplace_back(HotkeyMod_None, FDOV_SCANCODE_QUIT, 0, 0, Hotkey_Press, MsgType_Render_Quit);
    keymap->hotkeys.emplace_back(HotkeyMod_None, SDL_SCANCODE_ESCAPE, 0, 0, Hotkey_Press, MsgType_Render_Quit);
    keymap->hotkeys.emplace_back(HotkeyMod_None, SDL_SCANCODE_V, 0, 0, Hotkey_Press, MsgType_Render_ToggleVsync);
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
    int x = 0;
    int y = 0;
    SDL_GetMouseState(&x, &y);
    position->pos = { (float)x, (float)y };
    position->size = { 1, 1 };

    Keymap *keymap = (Keymap *)depot.AddFacet(uidCursor, Facet_Keymap);
    keymap->hotkeys.emplace_back(HotkeyMod_None, FDOV_SCANCODE_MOUSE_LEFT, 0, 0, Hotkey_Press, MsgType_Cursor_PrimaryPress);
    keymap->hotkeys.emplace_back(HotkeyMod_None, FDOV_SCANCODE_MOUSE_LEFT, 0, 0, Hotkey_Release | Hotkey_Handled, MsgType_Cursor_PrimaryRelease);

    return uidCursor;
}

struct Slice {
    const char * data   {};  // nil terminated, read-only
    size_t       length {};
};

enum Error {
    E_SUCCESS,         // no error
    E_EOF,             // reached EOF prematurely
    E_UNEXPECTED_CHAR, // unexpected character encountered
    E_IO_ERROR,        // failed to read/write file
    E_VERIFY_FAILED,   // for flatbuffers
};

Error load_resource_db(Depot &depot, const char *filename)
{
    size_t size{};
    void *data = SDL_LoadFile(filename, &size);
    if (!data) {
        return E_IO_ERROR;
    }

    const DB::ResourceDB *db = DB::GetResourceDB(data);
    flatbuffers::Verifier fbv((const u8 *)data, size);
    if (!DB::VerifyResourceDBBuffer(fbv)) {
        return E_VERIFY_FAILED;
    }

    printf("resdb %s\n", db->name()->c_str());
    for (const auto &dbSheet : *db->spritesheets()) {
        printf("  sheet %s has %d %dx%d cells \n",
            dbSheet->name()->c_str(),
            dbSheet->cell_count(),
            dbSheet->cell_width(),
            dbSheet->cell_height()
        );

        UID uidSheet = depot.Alloc(dbSheet->name()->c_str());
        Spritesheet *sheet = (Spritesheet *)depot.AddFacet(uidSheet,
            Facet_Spritesheet);

        sheet->texture = depot.renderSystem.LoadTexture_BMP(depot,
            dbSheet->texture_path()->c_str());

        sheet->cells = dbSheet->cell_count();
        sheet->cellSize.x = dbSheet->cell_width();
        sheet->cellSize.y = dbSheet->cell_height();

        for (const auto &dbAnim : *dbSheet->animations()) {
            printf("    anim %s start %d count %d\n",
                dbAnim->name()->c_str(),
                dbAnim->frame_start(),
                dbAnim->frame_count()
            );

            sheet->animations.push_back({
                dbAnim->name()->c_str(),
                dbAnim->desc()->c_str(),
                dbAnim->frame_start(),
                dbAnim->frame_count()
            });
            sheet->animations_by_name[dbAnim->name()->c_str()] = sheet->animations.size() - 1;
        }
    }

    // TODO: Cleanup resourceDB somewhere, or make copies of strings into
    // resourceArena, etc.
    //SDL_free(data);

    // Credit: whaatsuuup in twitch chat noticed this wasn't here. If you forget
    // return values in functions meant to return things, all sorts of nonsensical
    // bullshit occurs.
    return E_SUCCESS;
}

UID find_spritesheet(Depot &depot, const char *name)
{
    // Check if already loaded
    Spritesheet *existingSheet = (Spritesheet *)depot.GetFacetByName(name, Facet_Spritesheet);
    if (existingSheet) {
        return existingSheet->uid;
    }

    // TODO: Load on-demand? Would need to pass in db_name + spritesheet_name
    return 0;
}

UID create_narrator(Depot &depot, UID subject)
{
    UID uidNarrator = depot.Alloc("narrator");

    Position *position = (Position *)depot.AddFacet(uidNarrator, Facet_Position);
    int windowWidth = 0, windowHeight = 0;
    SDL_GetWindowSize(depot.renderSystem.Window(), &windowWidth, &windowHeight);
    position->pos.x = windowWidth / 2.0f;
    position->pos.y = 200.0f;

    Position *campPos = (Position *)depot.GetFacet(depot.card.front().uid, Facet_Position);
    if (campPos) {
        position->pos.x = campPos->pos.x - 100.0f;
        position->pos.y = campPos->pos.y - 100.0f;
    }

    Text *text = (Text *)depot.AddFacet(uidNarrator, Facet_Text);
#if 0
    text->font = depot.textSystem.LoadFont(depot, "font/KarminaBold.otf", 64);
    text->str = "15 Days";
#endif
#if 1
    position->pos.x = 10.0f;
    position->pos.y = 4.0f;
    text->font = depot.textSystem.LoadFont(depot, "font/KarminaBold.otf", 64);
    text->str =
        C_RED     "Red"
        C_GREEN   " Green"
        C_BLUE    " Blue"
        C_CYAN    " Cyan"
        C_MAGENTA " Magenta"
        C_YELLOW  " Yellow"
        C_WHITE   " White";
#endif
#if 0
    text->font = depot.textSystem.LoadFont(depot, "font/OpenSans-Bold.ttf", 20);
    text->str = "The`g camp`w is your home.\n"
        "Your adventure starts here.\n"
        "`r+10 health`w while in camp.";
#endif

    text->align = TextAlign_VBottom_HCenter;
    text->color = C255(COLOR_RED);

    depot.triggerSystem.Trigger_Audio_PlaySound(depot, uidNarrator, MsgType_Card_Notify_DragBegin, "audio/narrator_drag_begin.wav");
    depot.triggerSystem.Trigger_Audio_PlaySound(depot, uidNarrator, MsgType_Card_Notify_DragEnd, "audio/narrator_drag_end.wav");

    // Self triggers
    depot.triggerSystem.Trigger_Text_UpdateText(depot, subject, MsgType_Combat_Notify_IdleBegin,
        uidNarrator, "Neutral", C255(COLOR_GRAY_4));
    // Subject triggers
    depot.triggerSystem.Trigger_Text_UpdateText(depot, subject, MsgType_Combat_Notify_AttackBegin,
        uidNarrator, "Primary", C255(COLOR_RED));
    depot.triggerSystem.Trigger_Text_UpdateText(depot, subject, MsgType_Combat_Notify_DefendBegin,
        uidNarrator, "Secondary", C255(COLOR_DODGER));

    // TODO: NarratorSystem
    // - Check if position.pos + sprite.size outside of screen w/h
    //       Msg_Physics_Notify_Collide (uid=player)
    //         v
    //       NarrationEvent_LeaveScreen (uid=player)
    //         v
    //       Msg_Narrator_Says (uid=narrator, text="collided!")
    // - NarratorSystem::Update()
    //   - If delay timer elapsed, dequeue oldest item in the narrationQueue
    // - NarratorSystem::Draw(narratorQueue, drawList);
    //   - Generate draw commands for active text using the narrationQueue

    return uidNarrator;
}

void combat_try_attack(Depot &depot, const Message &msg, const Trigger &trigger, void *userData)
{
    Combat *combat = (Combat *)depot.GetFacet(trigger.message.uid, Facet_Combat);
    if (!combat) {
        return;
    }

    bool canAttack = combat->Idle();
    if (canAttack) {
        combat->attackStartedAt = depot.Now();
        combat->attackCooldown = 0.2;

        Message notifyAttack{};
        notifyAttack.uid = trigger.message.uid;
        notifyAttack.type = MsgType_Combat_Notify_AttackBegin;
        depot.msgQueue.push_back(notifyAttack);
    }
}
void combat_try_defend(Depot &depot, const Message &msg, const Trigger &trigger, void *userData)
{
    Combat *combat = (Combat *)depot.GetFacet(trigger.message.uid, Facet_Combat);
    if (!combat) {
        return;
    }

    bool canDefend = combat->Idle() || combat->Defending();
    if (canDefend) {
        combat->defendStartedAt = depot.Now();
        combat->defendCooldown = 0.6;

        Message notifyDefend{};
        notifyDefend.uid = trigger.message.uid;
        notifyDefend.type = MsgType_Combat_Notify_DefendBegin;
        depot.msgQueue.push_back(notifyDefend);
    }
}
void combat_try_idle(Depot &depot, const Message &msg, const Trigger &trigger, void *userData)
{
    Combat *combat = (Combat *)depot.GetFacet(trigger.message.uid, Facet_Combat);
    if (!combat) {
        return;
    }

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

    if (idleBegin) {
        Message notifyIdle{};
        notifyIdle.uid = combat->uid;
        notifyIdle.type = MsgType_Combat_Notify_IdleBegin;
        depot.msgQueue.push_back(notifyIdle);
    }
}

UID create_player(Depot &depot)
{
    UID uidSheet = find_spritesheet(depot, "sheet_player");
    Spritesheet *sheet = (Spritesheet * )depot.GetFacet(uidSheet, Facet_Spritesheet);

    UID uidPlayer = depot.Alloc("player");

    Position *position = (Position *)depot.AddFacet(uidPlayer, Facet_Position);
    position->size = sheet->cellSize;
    position->pos = {
        SCREEN_W / 2.0f - position->size.x / 2.0f,
        SCREEN_H / 2.0f - position->size.y / 2.0f,
    };

    depot.AddFacet(uidPlayer, Facet_Combat);
    Sprite *sprite = (Sprite *)depot.AddFacet(uidPlayer, Facet_Sprite);
    depot.spriteSystem.InitSprite(depot, *sprite, C255(COLOR_WHEAT), uidSheet, "player");

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
    debugText->font = depot.textSystem.LoadFont(depot, "font/OpenSans-Bold.ttf", 16);
    debugText->str = 0;
    debugText->align = TextAlign_VBottom_HCenter;
    debugText->color = C255(COLOR_WHITE);

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

    depot.triggerSystem.Trigger_Audio_PlaySound(depot, uidPlayer, MsgType_Combat_Notify_AttackBegin, "audio/primary.wav");
    depot.triggerSystem.Trigger_Audio_PlaySound(depot, uidPlayer, MsgType_Combat_Notify_DefendBegin, "audio/secondary.wav", false);
    depot.triggerSystem.Trigger_Audio_PlaySound(depot, uidPlayer, MsgType_Card_Notify_DragBegin, "audio/player_drag_begin.wav");
    depot.triggerSystem.Trigger_Audio_PlaySound(depot, uidPlayer, MsgType_Card_Notify_DragEnd, "audio/player_drag_end.wav");

    {
        TriggerList *triggerList = (TriggerList *)depot.AddFacet(uidPlayer, Facet_TriggerList, false);

        Trigger attackTrigger{};
        attackTrigger.trigger = MsgType_Combat_Primary;
        attackTrigger.message.uid = uidPlayer;
        attackTrigger.callback = combat_try_attack;
        triggerList->triggers.push_back(attackTrigger);

        Trigger defendTrigger{};
        defendTrigger.trigger = MsgType_Combat_Secondary;
        defendTrigger.message.uid = uidPlayer;
        defendTrigger.callback = combat_try_defend;
        triggerList->triggers.push_back(defendTrigger);

        Trigger idleTrigger{};
        idleTrigger.trigger = MsgType_Render_FrameBegin;
        idleTrigger.message.uid = uidPlayer;
        idleTrigger.callback = combat_try_idle;
        triggerList->triggers.push_back(idleTrigger);
    }

    create_narrator(depot, uidPlayer);

    return uidPlayer;
}

void fps_update_text(Depot &depot, const Message &msg, const Trigger &trigger, void *userData)
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
#if 1
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

        Message updateText{};
        updateText.uid = trigger.message.uid;
        updateText.type = MsgType_Text_UpdateText;
        updateText.data.text_updatetext.str = fpsCounterBuf;
        updateText.data.text_updatetext.color = C255(COLOR_WHITE);
        updateText.data.text_updatetext.offset.y = 20.0f;
        depot.msgQueue.push_back(updateText);
    } else {
        printf("WARN: Failed to allocate enough frame arena space for fps counter string\n");
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
    //text->font = depot.textSystem.LoadFont(depot, "font/ChivoMono-Bold.ttf", 16);
    //text->font = depot.textSystem.LoadFont(depot, "font/FiraCode-Bold.ttf", 16);
    text->font = depot.textSystem.LoadFont(depot, "font/OpenSans-Bold.ttf", 16);
    //text->font = depot.textSystem.LoadFont(depot, "font/pricedown_bl.ttf", 16);

    text->str = "00 fps (00.00 ms)";
    text->align = TextAlign_VTop_HLeft;
    text->color = C255(COLOR_WHITE);

    Histogram *histo = (Histogram *)depot.AddFacet(uidFpsCounter, Facet_Histogram);
    for (int i = 1; i <= 100; i++) {
        histo->values.push_back(i);
    };

    depot.triggerSystem.Trigger_Audio_PlaySound(depot, uidFpsCounter, MsgType_Card_Notify_DragBegin, "audio/drag_begin.wav");
    depot.triggerSystem.Trigger_Audio_PlaySound(depot, uidFpsCounter, MsgType_Card_Notify_DragEnd, "audio/drag_end.wav");

    {
        TriggerList *triggerList = (TriggerList *)depot.AddFacet(uidFpsCounter, Facet_TriggerList, false);

        Trigger updateTextTrigger{};
        updateTextTrigger.trigger = MsgType_Render_FrameBegin;
        updateTextTrigger.message.uid = uidFpsCounter;
        updateTextTrigger.callback = fps_update_text;
        triggerList->triggers.push_back(updateTextTrigger);
    }

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

UID create_effect_list(Depot &depot, const char *name)
{
    UID uidEffectList = depot.Alloc(name);
    depot.AddFacet(uidEffectList, Facet_EffectList);
    return uidEffectList;
}

void add_effect_to_effect_list(Depot &depot, UID uidFxList, Effect &effect)
{
    EffectList *effectList = (EffectList *)depot.GetFacet(uidFxList, Facet_EffectList);
    effectList->effects.push_back(effect);
}

UID create_material_proto(Depot &depot, const char *name)
{
    UID uidMaterialProto = depot.Alloc(name);
    depot.AddFacet(uidMaterialProto, Facet_MaterialProto);
    return uidMaterialProto;
}

void add_flag_to_material_proto(Depot &depot, UID uidMaterialProto, MaterialFlags flag)
{
    MaterialProto *materialProto = (MaterialProto *)depot.GetFacet(uidMaterialProto, Facet_MaterialProto);
    materialProto->flags.set(flag);
}

void create_cards(Depot &depot)
{
    UID uidCardSheet = find_spritesheet(depot, "sheet_cards");
    UID uidCampfireSheet = find_spritesheet(depot, "sheet_campfire");

    // Effects
    Effect fxIgnite{ .type = Effect_IgniteFlammable };
    Effect fxExtinguish{ .type = Effect_ExtinguishFlammable };

    // Effect lists
    UID uidFireFxList = create_effect_list(depot, "fire_fx");
    add_effect_to_effect_list(depot, uidFireFxList, fxIgnite);

    UID uidWaterFxList = create_effect_list(depot, "water_fx");
    add_effect_to_effect_list(depot, uidWaterFxList, fxExtinguish);

    // Materials
    UID uidFlammableMaterialProto = create_material_proto(depot, "flammable_material");
    add_flag_to_material_proto(depot, uidFlammableMaterialProto, MaterialFlag_Flammable);

    // Card prototypes
    UID uidLighterProto = depot.cardSystem.PrototypeCard(depot, "Lighter", 0, uidFireFxList, uidCardSheet, "card_lighter");
    UID uidBucketProto = depot.cardSystem.PrototypeCard(depot, "Water Bucket", 0, uidWaterFxList, uidCardSheet, "card_bucket");
    UID uidBombProto = depot.cardSystem.PrototypeCard(depot, "Bomb", 0, 0, uidCardSheet, "card_bomb");
    UID uidCampProto = depot.cardSystem.PrototypeCard(depot, "Camp", 0, 0, uidCardSheet, "card_camp");
    depot.triggerSystem.Trigger_Audio_PlaySound(depot, uidBombProto, MsgType_Card_Notify_DragUpdate, "audio/fuse_burning.wav", false);
    depot.triggerSystem.Trigger_Audio_StopSound(depot, uidBombProto, MsgType_Card_Notify_DragEnd, "audio/fuse_burning.wav");
    depot.triggerSystem.Trigger_Audio_PlaySound(depot, uidBombProto, MsgType_Card_Notify_DragEnd, "audio/explosion.wav", true);
    depot.triggerSystem.Trigger_Render_Screenshake(depot, uidBombProto, MsgType_Card_Notify_DragEnd, 6.0f, 200.0f, 0.5);
    UID uidCampfireProto = depot.cardSystem.PrototypeCard(depot, "Campfire", uidFlammableMaterialProto, 0, uidCampfireSheet, "unlit");

    // Decks
    depot.cardSystem.SpawnDeck(depot, { 600, 300, 0 }, uidCardSheet, "card_backface");

    // Cards
    UNUSED(uidLighterProto);
    UNUSED(uidBucketProto);
    UNUSED(uidBombProto);
    UNUSED(uidCampProto);
    //depot.cardSystem.SpawnCard(depot, uidLighterProto, { 700, 300, 0 });
    //depot.cardSystem.SpawnCard(depot, uidBucketProto, { 800, 300, 0 });
    //depot.cardSystem.SpawnCard(depot, uidBombProto, { 900, 300, 0 });
    depot.cardSystem.SpawnCard(depot, uidCampProto, { 900, 300, 0 });

    // TODO:
    // type_a  ,  type_b      , action
    // campfire,  water_bucket, extinguish
    // campfire,  lighter     , ignite

    // action    , require_flags, exclude_flags, state
    // extinguish, flammable    ,              , on_fire = false
    // ignite    , flammable    ,              , on_fire = true

    UID uidCampfire = depot.cardSystem.SpawnCard(depot, uidCampfireProto, { 200, 500, 0 });
    depot.triggerSystem.Trigger_Sprite_UpdateAnimation(depot, uidCampfire, MsgType_Effect_OnFireBegin, uidCampfire, "burning");
    depot.triggerSystem.Trigger_Sprite_UpdateAnimation(depot, uidCampfire, MsgType_Effect_OnFireEnd, uidCampfire, "unlit");
    depot.triggerSystem.Trigger_Audio_PlaySound(depot, uidCampfire, MsgType_Effect_OnFireBegin, "audio/fire_start.wav", true);
    // TODO: Stop all other sounds playing on this UID (e.g. iterate all sound_play triggers for sounds and stop them??)
    depot.triggerSystem.Trigger_Audio_StopSound(depot, uidCampfire, MsgType_Effect_OnFireBegin, "audio/fire_extinguish.wav");
    depot.triggerSystem.Trigger_Audio_PlaySound(depot, uidCampfire, MsgType_Effect_OnFireEnd, "audio/fire_extinguish.wav", true);
    // TODO: Stop all other sounds playing on this UID (e.g. iterate all sound_play triggers for sounds and stop them??)
    depot.triggerSystem.Trigger_Audio_StopSound(depot, uidCampfire, MsgType_Effect_OnFireEnd, "audio/fire_start.wav");
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

    Depot depot {};
    depot.Init(GameState_Play);

    err = depot.renderSystem.Init("15days", SCREEN_W, SCREEN_H);
    if (err) return err;

    if (TTF_Init() < 0) {
        SDL_Log("Couldn't initialize TTF: %s\n", TTF_GetError());
        return -1;
    }

    err = depot.audioSystem.Init();
    if (err) {
        SDL_Log("Failed to initalize audio subsystem\n");
    }

    dlb_rand32_seed(SDL_GetTicks64());

    load_resource_db(depot, "db/resourcedb.fbb");

    create_global_keymap(depot);
    create_cursor(depot);
    create_fps_counter(depot);
    create_cards(depot);
    create_player(depot);

    // Run the game
    depot.TransitionTo(GameState_Play);
    depot.Run();
    depot.Destroy();

    TTF_Quit();

    // SDL is currently reporting 1 unfreed alloc, but I haven't bothered to
    // try to find it yet.
    printf("SDL reported %d unfreed allocations\n", SDL_GetNumAllocations());
    //getchar();
    return 0;
}

#define DLB_MATH_IMPLEMENTATION
#include "dlb/dlb_math.h"
#define DLB_RAND_IMPLEMENTATION
#include "dlb/dlb_rand.h"
#define DLB_HASH_IMPLEMENTATION
#include "dlb/dlb_hash.h"
#define DLB_MURMUR3_IMPLEMENTATION
#include "dlb/dlb_murmur3.h"