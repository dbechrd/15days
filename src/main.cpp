#include "common/basic.h"
#include "common/message.h"
#include "facets/depot.h"

DLB_ASSERT_HANDLER(dlb_assert_callback) {
    printf("[%s:%u] %s\n", filename, line, expr);
}
dlb_assert_handler_def *dlb_assert_handler = dlb_assert_callback;

const char *debugFont = "font/OpenSans-Bold.ttf";

UID load_font(Depot &depot, const char *filename, int ptsize)
{
    size_t keyLen = strlen(filename) + 10;
    char *key = (char *)depot.frameArena.Alloc(keyLen);
    snprintf(key, keyLen, "%s?ptsize=%d", filename, ptsize);

    // Check if already loaded
    Font *existingFont = (Font *)depot.GetFacetByName(key, Facet_Font);
    if (existingFont) {
        return existingFont->uid;
    }

    // Load a new font
    UID uidFont = depot.Alloc(key);
    Font *font = (Font *)depot.AddFacet(uidFont, Facet_Font);
    font->filename = filename;
    font->ptsize = ptsize;
    font->ttf_font = TTF_OpenFont(filename, ptsize);
    if (!font->ttf_font) {
        SDL_LogError(0, "Failed to load font %s\n", filename);
    }

    return uidFont;
}


UID load_sound(Depot &depot, const char *filename)
{
    // Check if already loaded
    Sound *existingSound = (Sound *)depot.GetFacetByName(filename, Facet_Sound);
    if (existingSound) {
        return existingSound->uid;
    }

    // Load a new audio buffer
    UID uidSound = depot.Alloc(filename);
    Sound *sound = (Sound *)depot.AddFacet(uidSound, Facet_Sound);
    depot.audioSystem.InitSound(depot, *sound, filename);
    return uidSound;
}

UID load_bitmap(Depot &depot, const char *filename)
{
    // Check if already loaded
    Sound *existingTexture = (Sound *)depot.GetFacetByName(filename, Facet_Texture);
    if (existingTexture) {
        return existingTexture->uid;
    }

    // Load a new texture
    UID uidTexture = depot.Alloc(filename);
    Texture *texture = (Texture *)depot.AddFacet(uidTexture, Facet_Texture);
    depot.renderSystem.InitTexture(*texture, filename);
    return uidTexture;
}

void add_sound_play_trigger(Depot &depot, UID subject, MsgType msgType,
    const char *soundFile, bool override = true)
{
    TriggerList *triggerList = (TriggerList *)depot.AddFacet(subject, Facet_TriggerList, false);

    Trigger trigger{};
    trigger.trigger = msgType;
    trigger.message.uid = load_sound(depot, soundFile);
    trigger.message.type = MsgType_Audio_PlaySound;
    trigger.message.data.audio_playsound.override = override;
    triggerList->triggers.push_back(trigger);
}

void add_sound_stop_trigger(Depot &depot, UID subject, MsgType msgType,
    const char *soundFile)
{
    TriggerList *triggerList = (TriggerList *)depot.AddFacet(subject, Facet_TriggerList, false);

    Trigger trigger{};
    trigger.trigger = msgType;
    trigger.message.uid = load_sound(depot, soundFile);
    trigger.message.type = MsgType_Audio_StopSound;
    triggerList->triggers.push_back(trigger);
}

void add_screenshake_trigger(Depot &depot, UID subject, MsgType msgType,
    float amount, float freq, double duration)
{
    TriggerList *triggerList = (TriggerList *)depot.AddFacet(subject, Facet_TriggerList, false);

    Trigger trigger{};
    trigger.trigger = msgType;
    //trigger.message.uid = target_camera_uid;
    trigger.message.type = MsgType_Render_Screenshake;
    trigger.message.data.render_screenshake.amount = amount;
    trigger.message.data.render_screenshake.freq = freq;
    trigger.message.data.render_screenshake.duration = duration;
    triggerList->triggers.push_back(trigger);
}

void add_text_update_trigger(Depot &depot, UID src, MsgType msgType, UID dst,
    const char *str, vec4 color)
{
    TriggerList *triggerList = (TriggerList *)depot.AddFacet(src, Facet_TriggerList, false);

    Trigger trigger{};
    trigger.trigger = msgType;
    trigger.message.uid = dst;
    trigger.message.type = MsgType_Text_UpdateText;
    trigger.message.data.text_updatetext.str = str;
    trigger.message.data.text_updatetext.color = color;
    triggerList->triggers.push_back(trigger);
}

//void only_if_drag_landed_on_trigger_target(Depot &depot, const Message &msg,
//    const Trigger &trigger, void *userData)
//{
//    DLB_ASSERT(msg.type == MsgType_Card_Notify_DragEnd);
//
//    UID target = (UID)(size_t)userData;
//
//    if (msg.data.card_dragend.landedOn == target) {
//        depot.msgQueue.push_back(trigger.message);
//    }
//}

void add_animation_update_trigger(Depot &depot, UID src, MsgType msgType, UID dst,
    int animation, TriggerCallback callback = 0, void *userData = 0)
{
    TriggerList *triggerList = (TriggerList *)depot.AddFacet(src, Facet_TriggerList, false);

    Trigger trigger{};
    trigger.trigger = msgType;
    trigger.message.uid = dst;
    trigger.message.type = MsgType_Sprite_UpdateAnimation;
    trigger.message.data.sprite_updateanimation.animation = animation;
    trigger.callback = callback;
    trigger.userData = userData;
    triggerList->triggers.push_back(trigger);
}

void add_special_relay_trigger(Depot &depot, UID src, UID dst)
{
    TriggerList *triggerList = (TriggerList *)depot.AddFacet(src, Facet_TriggerList, false);

    Trigger relayTrigger{};
    relayTrigger.trigger = MsgType_Special_RelayAllMessages;
    relayTrigger.message.uid = dst;
    triggerList->triggers.push_back(relayTrigger);
}

//rect entity_bbox(Depot &depot, UID uid)
//{
//    rect bbox{};
//
//    Position *position = (Position *)depot.GetFacet(uid, Facet_Position);
//    if (position) {
//        bbox.x = position->pos.x;
//        bbox.y = position->pos.y - position->pos.z;
//        bbox.w = 1;
//        bbox.h = 1;
//    }
//    Sprite *sprite = (Sprite *)depot.GetFacet(uid, Facet_Sprite);
//    if (sprite) {
//        bbox.w = sprite->size.w;
//        bbox.h = sprite->size.h;
//    }
//    Text *text = (Text *)depot.GetFacet(uid, Facet_Text);
//    if (text) {
//        // TODO: Have to account for both the text offset (in pos check)
//        // as well as alignment offsets for this to work correctly. We
//        // should make Sprite->GetBBox() and Text->GetBBox() helpers or
//        // something.
//        Texture *texture = (Texture *)depot.GetFacet(uid, Facet_Texture);
//        if (texture) {
//            bbox.w = texture->size.w;
//            bbox.h = texture->size.h;
//        }
//    }
//
//    return bbox;
//}
//
//UID find_sprite_at_screen_pos(Depot &depot, UID uid, vec2 *offset)
//{
//    rect bboxA = entity_bbox(depot, uid);
//    if (!bboxA.w || !bboxA.h) {
//        return 0;
//    }
//
//    UID uidLandedOn = 0;
//    float maxDepth = 0;
//    for (Sprite &sprite : depot.sprite) {
//        if (sprite.uid == uid) {
//            continue;
//        }
//
//        rect bboxB = entity_bbox(depot, sprite.uid);
//        if (!bboxB.w || !bboxB.h) {
//            continue;
//        }
//
//        float depth = bboxB.y + bboxB.h;
//        if (depth < maxDepth) {
//            continue;
//        }
//
//        if (rect_intersect(&bboxA, &bboxB)) {
//            if (offset) {
//                *offset = { (float)(bboxA.x - bboxB.x), (float)(bboxA.y - bboxB.y) };
//            }
//            uidLandedOn = sprite.uid;
//            maxDepth = depth;
//        }
//    }
//    return uidLandedOn;
//}

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

UID card_spritesheet(Depot &depot)
{
    const char *filename = "texture/cards.bmp";
    // Check if already loaded
    Spritesheet *existingSheet = (Spritesheet *)depot.GetFacetByName(filename, Facet_Spritesheet);
    if (existingSheet) {
        return existingSheet->uid;
    }

    UID uidSheetCards = load_bitmap(depot, filename);
    Spritesheet *sheetCards = (Spritesheet *)depot.AddFacet(uidSheetCards, Facet_Spritesheet);
    sheetCards->cells = 10;
    sheetCards->cellSize = { 100, 150 };
    for (int i = 0; i < sheetCards->cells; i++) {
        sheetCards->animations.push_back({ i, 1 });
    }
    return uidSheetCards;
}

UID campfire_spritesheet(Depot &depot)
{
    const char *filename = "texture/campfire_small.bmp";
    // Check if already loaded
    Spritesheet *existingSheet = (Spritesheet *)depot.GetFacetByName(filename, Facet_Spritesheet);
    if (existingSheet) {
        return existingSheet->uid;
    }

    UID uidSheetCampfire = load_bitmap(depot, filename);
    Spritesheet *sheetCampfire = (Spritesheet *)depot.AddFacet(uidSheetCampfire, Facet_Spritesheet);
    sheetCampfire->cells = 9;
    sheetCampfire->cellSize = { 100, 150 };
    sheetCampfire->animations.push_back({ 0, 1 });
    sheetCampfire->animations.push_back({ 1, 8 });
    return uidSheetCampfire;
}

UID create_narrator(Depot &depot, UID subject)
{
    UID uidNarrator = depot.Alloc("narrator");

    Position *position = (Position *)depot.AddFacet(uidNarrator, Facet_Position);
    //int windowW = 0, windowH = 0;
    //SDL_GetWindowSize(renderSystem.window, &windowW, &windowH);
    //position->pos.x = windowW / 2.0f;
    //position->pos.y = 200.0f;
    position->pos.x = 10.0f;
    position->pos.y = 4.0f;

    Text *text = (Text *)depot.AddFacet(uidNarrator, Facet_Text);
    text->font = load_font(depot, "font/KarminaBold.otf", 64);
    text->str = "15 Days";
    text->align = TextAlign_VBottom_HCenter;
    text->color = C255(COLOR_WHITE);

    add_sound_play_trigger(depot, uidNarrator, MsgType_Card_Notify_DragBegin, "audio/narrator_drag_begin.wav");
    add_sound_play_trigger(depot, uidNarrator, MsgType_Card_Notify_DragEnd, "audio/narrator_drag_end.wav");

    // Self triggers
    add_text_update_trigger(depot, subject, MsgType_Combat_Notify_IdleBegin,
        uidNarrator, "Neutral", C255(COLOR_GRAY_4));
    // Subject triggers
    add_text_update_trigger(depot, subject, MsgType_Combat_Notify_AttackBegin,
        uidNarrator, "Primary", C255(COLOR_RED));
    add_text_update_trigger(depot, subject, MsgType_Combat_Notify_DefendBegin,
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
    UID uidSpritesheet = load_bitmap(depot, "texture/player.bmp");
    Spritesheet *spritesheet = (Spritesheet *)depot.AddFacet(uidSpritesheet, Facet_Spritesheet);
    spritesheet->cells = 1;
    spritesheet->cellSize = { 70, 140 };
    Animation animation{};
    animation.start = 0;
    animation.count = 1;
    spritesheet->animations.push_back(animation);

    UID uidPlayer = depot.Alloc("player");

    Position *position = (Position *)depot.AddFacet(uidPlayer, Facet_Position);
    position->size = spritesheet->cellSize;
    position->pos = {
        SCREEN_W / 2.0f - position->size.x / 2.0f,
        SCREEN_H / 2.0f - position->size.y / 2.0f,
    };

    depot.AddFacet(uidPlayer, Facet_Combat);
    Sprite *sprite = (Sprite *)depot.AddFacet(uidPlayer, Facet_Sprite);
    depot.spriteSystem.InitSprite(depot, *sprite, C255(COLOR_WHEAT), uidSpritesheet);

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
    debugText->font = load_font(depot, "font/OpenSans-Bold.ttf", 16);
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

    add_sound_play_trigger(depot, uidPlayer, MsgType_Combat_Notify_AttackBegin, "audio/primary.wav");
    add_sound_play_trigger(depot, uidPlayer, MsgType_Combat_Notify_DefendBegin, "audio/secondary.wav", false);
    add_sound_play_trigger(depot, uidPlayer, MsgType_Card_Notify_DragBegin, "audio/player_drag_begin.wav");
    add_sound_play_trigger(depot, uidPlayer, MsgType_Card_Notify_DragEnd, "audio/player_drag_end.wav");

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
    //text->font = load_font(depot, "font/ChivoMono-Bold.ttf", 16);
    //text->font = load_font(depot, "font/FiraCode-Bold.ttf", 16);
    text->font = load_font(depot, "font/OpenSans-Bold.ttf", 16);

    text->str = "00 fps (00.00 ms)";
    text->align = TextAlign_VTop_HLeft;
    text->color = C255(COLOR_WHITE);

    Histogram *histo = (Histogram *)depot.AddFacet(uidFpsCounter, Facet_Histogram);
    for (int i = 1; i <= 100; i++) {
        histo->values.push_back(i);
    };

    add_sound_play_trigger(depot, uidFpsCounter, MsgType_Card_Notify_DragBegin, "audio/drag_begin.wav");
    add_sound_play_trigger(depot, uidFpsCounter, MsgType_Card_Notify_DragEnd, "audio/drag_end.wav");

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

UID draggable_sounds(Depot &depot)
{
    const char *name = "draggable_sounds";

    // Check if already loaded
    if (depot.uidByName.contains(name)) {
        return depot.uidByName[name];
    }

    UID uidDraggableSounds = depot.Alloc(name);
    add_sound_play_trigger(depot, uidDraggableSounds, MsgType_Card_Notify_DragBegin, "audio/drag_begin.wav");
    add_sound_play_trigger(depot, uidDraggableSounds, MsgType_Card_Notify_DragEnd, "audio/drag_end.wav");
    return uidDraggableSounds;
}

UID create_card_proto(Depot &depot, const char *name, UID uidMaterialProto,
    UID uidEffectList, UID spritesheet, int animation)
{
    UID uidCardProto = depot.Alloc(name);
    CardProto *cardProto = (CardProto *)depot.AddFacet(uidCardProto, Facet_CardProto);
    cardProto->materialProto = uidMaterialProto;
    cardProto->effectList = uidEffectList;
    cardProto->spritesheet = spritesheet;
    cardProto->animation = animation;

    add_special_relay_trigger(depot, uidCardProto, draggable_sounds(depot));

    return uidCardProto;
}

UID create_card(Depot &depot, UID uidCardProto, vec3 pos, double invulnFor = 0)
{
    UID uidCard = depot.Alloc(depot.nameByUid[uidCardProto].c_str(), false);

    Position *position = (Position *)depot.AddFacet(uidCard, Facet_Position);
    position->pos = pos;

    Card *card = (Card *)depot.AddFacet(uidCard, Facet_Card);
    card->cardProto = uidCardProto;
    card->noClickUntil = depot.Now() + invulnFor;

    CardProto *cardProto = (CardProto *)depot.GetFacet(uidCardProto, Facet_CardProto);
    if (cardProto->materialProto) {
        Material *material = (Material *)depot.AddFacet(uidCard, Facet_Material);
        material->materialProto = cardProto->materialProto;
    }

    Spritesheet *sheet = (Spritesheet *)depot.GetFacet(cardProto->spritesheet, Facet_Spritesheet);
    if (sheet) {
        Sprite *sprite = (Sprite *)depot.AddFacet(uidCard, Facet_Sprite);
        SpriteSystem::InitSprite(depot, *sprite, C255(COLOR_WHITE), cardProto->spritesheet);
        sprite->SetSpritesheet(depot, cardProto->spritesheet);
        sprite->SetAnimIndex(depot, cardProto->animation);
    } else {
        DLB_ASSERT(!"no sheet");
        printf("Failed to find sheet for card\n");
    }

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
    debugText->font = load_font(depot, "font/OpenSans-Bold.ttf", 16);
    debugText->str = 0;
    debugText->align = TextAlign_VBottom_HCenter;
    debugText->color = C255(COLOR_WHITE);

    add_special_relay_trigger(depot, uidCard, uidCardProto);

    return uidCard;
}

void deck_draw_card(Depot &depot, const Message &msg, const Trigger &trigger, void *userData)
{
    UID uid = msg.uid;
    Deck *deck = (Deck *)depot.GetFacet(uid, Facet_Deck);
    if (!deck) {
        return;
    }

#if 0
    while (deck->count) {
#else
    if (deck->count) {
#endif
        vec3 spawnPos{};
        Position *position = (Position *)depot.GetFacet(uid, Facet_Position);
        if (position) {
            spawnPos = position->pos;
            spawnPos.y += 50;  // TODO: Rand pop
        }
        Sprite *sprite = (Sprite *)depot.GetFacet(uid, Facet_Sprite);
        if (sprite) {
            // TODO: Pick cardProto from deck chances
            UID cardProto = depot.cardProto[dlb_rand32i_range(0, 2)].uid;
            UID card = create_card(depot, cardProto, spawnPos, 0.5);

            Body *body = (Body *)depot.GetFacet(card, Facet_Body);
            body->impulseBuffer.x = dlb_rand32f_range(0.0f, 1.0f) * (dlb_rand32i_range(0, 1) ? 1.0f : -1.0f);
            body->impulseBuffer.y = dlb_rand32f_range(0.0f, 1.0f) * (dlb_rand32i_range(0, 1) ? 1.0f : -1.0f);
            v3_scalef(v3_normalize(&body->impulseBuffer), dlb_rand32f_range(800.0f, 1200.0f));
            body->jumpBuffer = 12.0f + dlb_rand32f_variance(2.0f);
            depot.renderSystem.Shake(depot, 3.0f, 100.0f, 0.1f);
        } else {
            SDL_Log("Cannot draw from deck with no spritesheet\n");
        }
        deck->count--;
    }

    if (!deck->count) {
        // TODO: Destroy the deck
        // TODO: Remove "Trigger" as a Facet and go back to std::vector<Trigger> inside of TriggerList??
    }
}

UID create_deck(Depot &depot, vec3 pos, UID spritesheet, int animation)
{
    UID uidDeck = depot.Alloc("deck", false);

    Position *position = (Position *)depot.AddFacet(uidDeck, Facet_Position);
    position->pos = pos;

    Deck *deck = (Deck *)depot.AddFacet(uidDeck, Facet_Deck);
    deck->count = 100;

    Sprite *sprite = (Sprite *)depot.AddFacet(uidDeck, Facet_Sprite);
    SpriteSystem::InitSprite(depot, *sprite, C255(COLOR_WHITE), spritesheet);
    sprite->SetSpritesheet(depot, spritesheet);
    sprite->SetAnimIndex(depot, animation);

    Body *body = (Body *)depot.AddFacet(uidDeck, Facet_Body);
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

    Text *debugText = (Text *)depot.AddFacet(uidDeck, Facet_Text);
    debugText->font = load_font(depot, "font/OpenSans-Bold.ttf", 16);
    debugText->str = 0;
    debugText->align = TextAlign_VBottom_HCenter;
    debugText->color = C255(COLOR_WHITE);

    add_special_relay_trigger(depot, uidDeck, draggable_sounds(depot));

    TriggerList *triggerList = (TriggerList *)depot.AddFacet(uidDeck, Facet_TriggerList, false);

    Trigger deckDrawTrigger{};
    deckDrawTrigger.trigger = MsgType_Card_Notify_LeftQuickClick;
    deckDrawTrigger.message.uid = uidDeck;
    deckDrawTrigger.callback = deck_draw_card;
    triggerList->triggers.push_back(deckDrawTrigger);

    return uidDeck;
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

    // TODO: Use dlb_rand or something
    dlb_rand32_seed(SDL_GetTicks64());

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

    // TODO: Resource loader (maybe RenderSystem::LoadFont()?)
    // https://github.com/grimfang4/SDL_FontCache
    // https://github.com/libsdl-org/SDL_ttf/blob/main/showfont.c

    // Create an entity to hold the global keymap (the plan is to have a global
    // keymap per gamestate eventually)
    create_global_keymap(depot);
    create_cursor(depot);

    // Create player/narrator
    create_player(depot);
    create_fps_counter(depot);

    {
        UID uidCardSheet = card_spritesheet(depot);
        UID uidCampfireSheet = campfire_spritesheet(depot);

        // Effects
        Effect fxIgnite     { .type = Effect_IgniteFlammable };
        Effect fxExtinguish { .type = Effect_ExtinguishFlammable };

        // Effect lists
        UID uidFireFxList = create_effect_list(depot, "fire_fx");
        add_effect_to_effect_list(depot, uidFireFxList, fxIgnite);

        UID uidWaterFxList = create_effect_list(depot, "water_fx");
        add_effect_to_effect_list(depot, uidWaterFxList, fxExtinguish);

        // Materials
        UID uidFlammableMaterialProto = create_material_proto(depot, "flammable_material");
        add_flag_to_material_proto(depot, uidFlammableMaterialProto, MaterialFlag_Flammable);

        // Card prototypes
        UID uidLighterProto = create_card_proto(depot, "lighter_card", 0, uidFireFxList, uidCardSheet, 0);
        UID uidBucketProto = create_card_proto(depot, "bucket_card", 0, uidWaterFxList, uidCardSheet, 1);
        UID uidBombProto = create_card_proto(depot, "bomb_card", 0, 0, uidCardSheet, 3);
        add_sound_play_trigger(depot, uidBombProto, MsgType_Card_Notify_DragUpdate, "audio/fuse_burning.wav", false);
        add_sound_stop_trigger(depot, uidBombProto, MsgType_Card_Notify_DragEnd, "audio/fuse_burning.wav");
        add_sound_play_trigger(depot, uidBombProto, MsgType_Card_Notify_DragEnd, "audio/explosion.wav", true);
        add_screenshake_trigger(depot, uidBombProto, MsgType_Card_Notify_DragEnd, 6.0f, 200.0f, 0.5);
        UID uidCampfireProto = create_card_proto(depot, "campfire_card", uidFlammableMaterialProto, 0, uidCampfireSheet, 0);

        // Decks
        create_deck(depot, { 600, 300, 0 }, uidCardSheet, 2);

        // Cards
        create_card(depot, uidLighterProto, { 700, 300, 0 });
        create_card(depot, uidBucketProto, { 800, 300, 0 });
        create_card(depot, uidBombProto, { 900, 300, 0 });

        // TODO:
        // type_a  ,  type_b      , action
        // campfire,  water_bucket, extinguish
        // campfire,  lighter     , ignite

        // action    , require_flags, exclude_flags, state
        // extinguish, flammable    ,              , on_fire = false
        // ignite    , flammable    ,              , on_fire = true

        UID uidCampfire = create_card(depot, uidCampfireProto, { 200, 500, 0 });
        add_animation_update_trigger(depot, uidCampfire, MsgType_Effect_OnFireBegin, uidCampfire, 1);
        add_animation_update_trigger(depot, uidCampfire, MsgType_Effect_OnFireEnd, uidCampfire, 0);
        add_sound_play_trigger(depot, uidCampfire, MsgType_Effect_OnFireBegin, "audio/fire_start.wav", true);
        // TODO: Stop all other sounds playing on this UID (e.g. iterate all sound_play triggers for sounds and stop them??)
        add_sound_stop_trigger(depot, uidCampfire, MsgType_Effect_OnFireBegin, "audio/fire_extinguish.wav");
        add_sound_play_trigger(depot, uidCampfire, MsgType_Effect_OnFireEnd, "audio/fire_extinguish.wav", true);
        // TODO: Stop all other sounds playing on this UID (e.g. iterate all sound_play triggers for sounds and stop them??)
        add_sound_stop_trigger(depot, uidCampfire, MsgType_Effect_OnFireEnd, "audio/fire_start.wav");
    }

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