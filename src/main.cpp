#include "common/message.h"
#include "facets/depot.h"
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"
#include <cassert>
#include <cstdio>
#include <string>

DLB_ASSERT_HANDLER(dlb_assert_callback) {
    printf("[%s:%u] %s\n", filename, line, expr);
}
dlb_assert_handler_def *dlb_assert_handler = dlb_assert_callback;

UID load_font(Depot &depot, const char *filename, int ptsize)
{
    size_t keyLen = strlen(filename) + 10;
    char *key = (char *)depot.frameArena.Alloc(keyLen);
    snprintf(key, keyLen, "%s?ptsize=%d", filename, ptsize);

    // Check if already loaded
    Font *existingFont = (Font *)depot.GetFacetByName(Facet_Font, key);
    if (existingFont) {
        return existingFont->uid;
    }

    // Load a new font
    UID uidFont = depot.Alloc();
    Font *fontFancy = (Font *)depot.AddFacet(uidFont, Facet_Font, key);
    fontFancy->filename = filename;
    fontFancy->ptsize = ptsize;
    fontFancy->ttf_font = TTF_OpenFont(filename, ptsize);
    return uidFont;
}


UID load_sound(Depot &depot, const char *filename)
{
    // Check if already loaded
    Sound *existingSound = (Sound *)depot.GetFacetByName(Facet_Sound, filename);
    if (existingSound) {
        return existingSound->uid;
    }

    // Load a new audio buffer
    UID uidSound = depot.Alloc();
    printf("%u: sound %s\n", uidSound, filename);
    Sound *sound = (Sound *)depot.AddFacet(uidSound, Facet_Sound, filename);
    depot.audioSystem.InitSound(depot, *sound, filename);
    return uidSound;
}

UID load_bitmap(Depot &depot, const char *filename)
{
    // Check if already loaded
    Sound *existingTexture = (Sound *)depot.GetFacetByName(Facet_Texture, filename);
    if (existingTexture) {
        return existingTexture->uid;
    }

    // Load a new texture
    UID uidTexture = depot.Alloc();
    Texture *texture = (Texture *)depot.AddFacet(uidTexture, Facet_Texture);
    depot.renderSystem.InitTexture(*texture, filename);
    return uidTexture;
}

void add_sound_trigger(Depot &depot, UID subject, MsgType msgType,
    const char *soundFile, bool override = true, TriggerCallback callback = 0,
    void *userData = 0)
{
    UID uidAudioBuffer = load_sound(depot, soundFile);

    UID uidTrigger = depot.Alloc();
    printf("%u: trigger on %u to play sound %u\n", uidTrigger, subject, uidAudioBuffer);
    Trigger *trigger = (Trigger *)depot.AddFacet(uidTrigger, Facet_Trigger);
    trigger->trigger = msgType;
    trigger->message.uid = uidAudioBuffer;
    trigger->message.type = MsgType_Audio_PlaySound;
    trigger->message.data.audio_playsound.override = override;
    trigger->callback = callback;
    trigger->userData = userData;

    TriggerList *triggerList = (TriggerList *)depot.AddFacet(subject, Facet_TriggerList, 0, false);
    triggerList->triggers.insert(uidTrigger);
}

void add_text_update_trigger(Depot &depot, UID src, MsgType msgType, UID dst,
    const char *str, vec4 color)
{
    UID uidTrigger = depot.Alloc();
    printf("%u: trigger on %u to update text for %u\n", uidTrigger, src, dst);
    Trigger *trigger = (Trigger *)depot.AddFacet(uidTrigger, Facet_Trigger);
    trigger->trigger = msgType;
    trigger->message.uid = dst;
    trigger->message.type = MsgType_Text_UpdateText;
    trigger->message.data.text_updatetext.str = str;
    trigger->message.data.text_updatetext.color = color;

    TriggerList *triggerList = (TriggerList *)depot.AddFacet(src, Facet_TriggerList, 0, false);
    triggerList->triggers.insert(uidTrigger);
}


void only_if_drag_landed_on_trigger_target(Depot &depot, const Message &msg,
    const Trigger &trigger, void *userData)
{
    DLB_ASSERT(msg.type == MsgType_Card_Notify_DragEnd);

    UID target = (UID)(size_t)userData;

    if (msg.data.card_dragend.landedOn == target) {
        depot.msgQueue.push_back(trigger.message);
    }
}

void add_animation_update_trigger(Depot &depot, UID src, MsgType msgType, UID dst,
    int animation, TriggerCallback callback = 0, void *userData = 0)
{
    UID uidTrigger = depot.Alloc();
    printf("%u: trigger on %u to update animation for %u\n", uidTrigger, src, dst);
    Trigger *trigger = (Trigger *)depot.AddFacet(uidTrigger, Facet_Trigger);
    trigger->trigger = msgType;
    trigger->message.uid = dst;
    trigger->message.type = MsgType_Sprite_UpdateAnimation;
    trigger->message.data.sprite_updateanimation.animation = animation;
    trigger->callback = callback;
    trigger->userData = userData;

    TriggerList *triggerList = (TriggerList *)depot.AddFacet(src, Facet_TriggerList, 0, false);
    triggerList->triggers.insert(uidTrigger);
}

rect entity_bbox(Depot &depot, UID uid)
{
    rect bbox{};

    Position *position = (Position *)depot.GetFacet(uid, Facet_Position);
    if (position) {
        bbox.x = position->pos.x;
        bbox.y = position->pos.y - position->pos.z;
        bbox.w = 1;
        bbox.h = 1;
    }
    Sprite *sprite = (Sprite *)depot.GetFacet(uid, Facet_Sprite);
    if (sprite) {
        bbox.w = sprite->size.w;
        bbox.h = sprite->size.h;
    }
    Text *text = (Text *)depot.GetFacet(uid, Facet_Text);
    if (text) {
        // TODO: Have to account for both the text offset (in pos check)
        // as well as alignment offsets for this to work correctly. We
        // should make Sprite->GetBBox() and Text->GetBBox() helpers or
        // something.
        Texture *texture = (Texture *)depot.GetFacet(uid, Facet_Texture);
        if (texture) {
            bbox.w = texture->size.w;
            bbox.h = texture->size.h;
        }
    }

    return bbox;
}

UID find_sprite_at_screen_pos(Depot &depot, UID uid, vec2 *offset)
{
    rect bboxA = entity_bbox(depot, uid);
    if (!bboxA.w || !bboxA.h) {
        return 0;
    }

    UID uidLandedOn = 0;
    float maxDepth = 0;
    for (Sprite &sprite : depot.sprite) {
        if (sprite.uid == uid) {
            continue;
        }

        rect bboxB = entity_bbox(depot, sprite.uid);
        if (!bboxB.w || !bboxB.h) {
            continue;
        }

        float depth = bboxB.y + bboxB.h;
        if (depth < maxDepth) {
            continue;
        }

        if (rect_intersect(&bboxA, &bboxB)) {
            if (offset) {
                *offset = { (float)(bboxA.x - bboxB.x), (float)(bboxA.y - bboxB.y) };
            }
            uidLandedOn = sprite.uid;
            maxDepth = depth;
        }
    }
    return uidLandedOn;
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

void cursor_try_drag_begin(Depot &depot, const Message &msg, const Trigger &trigger, void *userData)
{
    Cursor *cursor = (Cursor *)depot.GetFacet(msg.uid, Facet_Cursor);
    if (!cursor) {
        return;
    }

    cursor->uidDragSubject = find_sprite_at_screen_pos(depot, cursor->uid, &cursor->dragOffset);
    if (cursor->uidDragSubject) {
        Message dragBegin{};
        dragBegin.type = MsgType_Card_Notify_DragBegin;
        dragBegin.uid = cursor->uidDragSubject;
        depot.msgQueue.push_back(dragBegin);
    }
}
void cursor_try_drag_update(Depot &depot, const Message &msg, const Trigger &trigger, void *userData)
{
    UID uid = (UID)(size_t)userData;
    Cursor *cursor = (Cursor *)depot.GetFacet(uid, Facet_Cursor);
    if (!cursor) {
        return;
    }

    if (cursor->uidDragSubject) {
        Position *position = (Position *)depot.GetFacet(uid, Facet_Position);
        DLB_ASSERT(position);
        if (!position) {
            printf("WARN: Can't use a cursor with no position");
            return;
        }

        // TODO: This should probably generate a message instead.. like ApplyImpulse
        Position *subjectPos = (Position *)depot.GetFacet(cursor->uidDragSubject, Facet_Position);
        if (subjectPos) {
            subjectPos->pos.x = (float)position->pos.x - cursor->dragOffset.x;
            subjectPos->pos.y = (float)position->pos.y - cursor->dragOffset.y;
        }

        //Message dragUpdate{};
        //dragUpdate.type = MsgType_Physics_ApplyImpulse;
        //dragUpdate.uid = cursor->uidDragSubject;
        //depot.msgQueue.push_back(dragUpdate);
    }
}
void cursor_try_drag_end(Depot &depot, const Message &msg, const Trigger &trigger, void *userData)
{
    Cursor *cursor = (Cursor *)depot.GetFacet(msg.uid, Facet_Cursor);
    if (!cursor) {
        return;
    }

    if (cursor->uidDragSubject) {
        UID landedOn = find_sprite_at_screen_pos(depot, cursor->uidDragSubject, &cursor->dragOffset);

        Message dragEnd{};
        dragEnd.type = MsgType_Card_Notify_DragEnd;
        dragEnd.uid = cursor->uidDragSubject;
        dragEnd.data.card_dragend.landedOn = landedOn;
        depot.msgQueue.push_back(dragEnd);

        cursor->uidDragSubject = 0;
    }
}
UID create_cursor(Depot &depot)
{
    UID uidCursor = depot.Alloc();
    printf("%u: cursor\n", uidCursor);

    Cursor *cursor = (Cursor *)depot.AddFacet(uidCursor, Facet_Cursor);
    UNUSED(cursor);

    Position *position = (Position *)depot.AddFacet(uidCursor, Facet_Position);
    int x = 0;
    int y = 0;
    SDL_GetMouseState(&x, &y);
    position->pos.x = (float)x;
    position->pos.y = (float)y;

    Keymap *keymap = (Keymap *)depot.AddFacet(uidCursor, Facet_Keymap);
    keymap->hotkeys.emplace_back(HotkeyMod_None, FDOV_SCANCODE_MOUSE_LEFT, 0, 0, Hotkey_Press, MsgType_Cursor_PrimaryPress);
    keymap->hotkeys.emplace_back(HotkeyMod_None, FDOV_SCANCODE_MOUSE_LEFT, 0, 0, Hotkey_Release | Hotkey_Handled, MsgType_Cursor_PrimaryRelease);

    {
        UID uidDragBeginTrigger = depot.Alloc();
        Trigger *dragBeginTrigger = (Trigger *)depot.AddFacet(uidDragBeginTrigger, Facet_Trigger);
        dragBeginTrigger->trigger = MsgType_Cursor_PrimaryPress;
        dragBeginTrigger->callback = cursor_try_drag_begin;

        UID uidDragUpdateTrigger = depot.Alloc();
        Trigger *dragUpdateTrigger = (Trigger *)depot.AddFacet(uidDragUpdateTrigger, Facet_Trigger);
        dragUpdateTrigger->trigger = MsgType_Render_FrameBegin;
        dragUpdateTrigger->callback = cursor_try_drag_update;
        dragUpdateTrigger->userData = (void *)(size_t)uidCursor;

        UID uidDragEndTrigger = depot.Alloc();
        Trigger *dragEndTrigger = (Trigger *)depot.AddFacet(uidDragEndTrigger, Facet_Trigger);
        dragEndTrigger->trigger = MsgType_Cursor_PrimaryRelease;
        dragEndTrigger->callback = cursor_try_drag_end;

        TriggerList *triggerList = (TriggerList *)depot.AddFacet(uidCursor, Facet_TriggerList, 0, false);
        triggerList->triggers.insert(uidDragBeginTrigger);
        triggerList->triggers.insert(uidDragUpdateTrigger);
        triggerList->triggers.insert(uidDragEndTrigger);
    }

    return uidCursor;
}

UID create_narrator(Depot &depot, UID subject)
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
    text->font = load_font(depot, "font/KarminaBold.otf", 64);
    text->str = "15 Days";
    text->align = TextAlign_VBottom_HCenter;
    text->color = C255(COLOR_WHITE);
    text->dirty = true;

    add_sound_trigger(depot, uidNarrator, MsgType_Card_Notify_DragBegin, "audio/narrator_drag_begin.wav");
    add_sound_trigger(depot, uidNarrator, MsgType_Card_Notify_DragEnd, "audio/narrator_drag_end.wav");

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
        combat->defendCooldown = 5.0; //0.6;

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

    UID uidPlayer = depot.Alloc();
    printf("%u: player\n", uidPlayer);

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

    depot.AddFacet(uidPlayer, Facet_Combat);
    Sprite *sprite = (Sprite *)depot.AddFacet(uidPlayer, Facet_Sprite);
    depot.spriteSystem.InitSprite(*sprite, spritesheet->cellSize, C255(COLOR_WHEAT));
    sprite->spritesheet = uidSpritesheet;
    sprite->animation = 0;
    sprite->frame = 0;

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
    debugText->font = load_font(depot, "font/Hack-Bold.ttf", 16);;
    debugText->str = 0;
    debugText->align = TextAlign_VBottom_HCenter;
    debugText->color = C255(COLOR_WHITE);

    add_sound_trigger(depot, uidPlayer, MsgType_Combat_Notify_AttackBegin, "audio/primary.wav");
    add_sound_trigger(depot, uidPlayer, MsgType_Combat_Notify_DefendBegin, "audio/secondary.wav", false);
    add_sound_trigger(depot, uidPlayer, MsgType_Card_Notify_DragBegin, "audio/player_drag_begin.wav");
    add_sound_trigger(depot, uidPlayer, MsgType_Card_Notify_DragEnd, "audio/player_drag_end.wav");

    {
        UID uidAttackTrigger = depot.Alloc();
        Trigger *attackTrigger = (Trigger *)depot.AddFacet(uidAttackTrigger, Facet_Trigger);
        attackTrigger->trigger = MsgType_Combat_Primary;
        attackTrigger->message.uid = uidPlayer;
        attackTrigger->callback = combat_try_attack;

        UID uidDefendTrigger = depot.Alloc();
        Trigger *defendTrigger = (Trigger *)depot.AddFacet(uidDefendTrigger, Facet_Trigger);
        defendTrigger->trigger = MsgType_Combat_Secondary;
        defendTrigger->message.uid = uidPlayer;
        defendTrigger->callback = combat_try_defend;

        UID uidIdleTrigger = depot.Alloc();
        Trigger *idleTrigger = (Trigger *)depot.AddFacet(uidIdleTrigger, Facet_Trigger);
        idleTrigger->trigger = MsgType_Render_FrameBegin;
        idleTrigger->message.uid = uidPlayer;
        idleTrigger->callback = combat_try_idle;

        TriggerList *triggerList = (TriggerList *)depot.AddFacet(uidPlayer, Facet_TriggerList, 0, false);
        triggerList->triggers.insert(uidAttackTrigger);
        triggerList->triggers.insert(uidDefendTrigger);
        triggerList->triggers.insert(uidIdleTrigger);
    }

    create_narrator(depot, uidPlayer);

    return uidPlayer;
}

void fps_update_text(Depot &depot, const Message &msg, const Trigger &trigger, void *userData)
{
    const double dt = depot.DtSmooth();
    const double fps = 1.0f / dt;
    const double dtMillis = dt * 1000.0f;
    const size_t fpsCounterMaxLen = 32;

    char *fpsCounterBuf = (char *)depot.frameArena.Alloc(fpsCounterMaxLen);
    if (fpsCounterBuf) {
        snprintf(fpsCounterBuf, fpsCounterMaxLen, "%.2f fps (%.2f ms)", fps, dtMillis);

        Message updateText{};
        updateText.uid = trigger.message.uid;
        updateText.type = MsgType_Text_UpdateText;
        updateText.data.text_updatetext.str = fpsCounterBuf;
        updateText.data.text_updatetext.color = C255(COLOR_WHITE);
        depot.msgQueue.push_back(updateText);
    } else {
        printf("WARN: Failed to allocate enough frame arena space for fps counter string\n");
    }
}

UID create_fps_counter(Depot &depot)
{
    UID uidFpsCounter = depot.Alloc();
    printf("%u: fps counter\n", uidFpsCounter);

    FpsCounter *fpsCounter = (FpsCounter *)depot.AddFacet(uidFpsCounter, Facet_FpsCounter);
    UNUSED(fpsCounter);

    Position *position = (Position *)depot.AddFacet(uidFpsCounter, Facet_Position);
    //int windowW = 0, windowH = 0;
    //SDL_GetWindowSize(renderSystem.window, &windowW, &windowH);
    //position->pos.x = windowW / 2.0f;
    //position->pos.y = 200.0f;
    position->pos.x = 10;
    position->pos.y = 10;

    Text *text = (Text *)depot.AddFacet(uidFpsCounter, Facet_Text);
    text->font = load_font(depot, "font/Hack-Bold.ttf", 16);
    text->str = "00 fps (00.00 ms)";
    text->align = TextAlign_VTop_HLeft;
    text->color = C255(COLOR_WHITE);

    add_sound_trigger(depot, uidFpsCounter, MsgType_Card_Notify_DragBegin, "audio/drag_begin.wav");
    add_sound_trigger(depot, uidFpsCounter, MsgType_Card_Notify_DragEnd, "audio/drag_end.wav");

    {
        UID uidUpdateTextTrigger = depot.Alloc();
        Trigger *updateTextTrigger = (Trigger *)depot.AddFacet(uidUpdateTextTrigger, Facet_Trigger);
        updateTextTrigger->trigger = MsgType_Render_FrameBegin;
        updateTextTrigger->message.uid = uidFpsCounter;
        updateTextTrigger->callback = fps_update_text;

        TriggerList *triggerList = (TriggerList *)depot.AddFacet(uidFpsCounter, Facet_TriggerList, 0, false);
        triggerList->triggers.insert(uidUpdateTextTrigger);
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

UID create_card(Depot &depot, vec3 pos, UID spritesheet, int animation)
{
    UID uidCard = depot.Alloc();
    printf("%u: card\n", uidCard);

    Spritesheet *sheet = (Spritesheet *)depot.GetFacet(spritesheet, Facet_Spritesheet);
    Sprite *sprite = (Sprite *)depot.AddFacet(uidCard, Facet_Sprite);
    SpriteSystem::InitSprite(*sprite, sheet->cellSize, C255(COLOR_WHITE));
    sprite->spritesheet = spritesheet;
    sprite->animation = animation;
    sprite->frame = 0;

    Position *position = (Position *)depot.AddFacet(uidCard, Facet_Position);
    position->pos = pos;

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
    debugText->font = load_font(depot, "font/Hack-Bold.ttf", 16);
    debugText->str = 0;
    debugText->align = TextAlign_VBottom_HCenter;
    debugText->color = C255(COLOR_WHITE);

    add_sound_trigger(depot, uidCard, MsgType_Card_Notify_DragBegin, "audio/drag_begin.wav");
    add_sound_trigger(depot, uidCard, MsgType_Card_Notify_DragEnd, "audio/drag_end.wav");

    return uidCard;
}

void deck_draw(Depot &depot, const Message &msg, const Trigger &trigger, void *userData)
{
    UID uid = (UID)(size_t)userData;
    Deck *deck = (Deck *)depot.GetFacet(uid, Facet_Deck);
    if (!deck) {
        return;
    }

    if (deck->count) {
        vec3 spawnPos{};
        Position *position = (Position *)depot.GetFacet(uid, Facet_Position);
        if (position) {
            spawnPos = position->pos;
            spawnPos.y += 50;  // TODO: Rand pop
        }
        Sprite *sprite = (Sprite *)depot.GetFacet(uid, Facet_Sprite);
        if (sprite) {
            create_card(depot, spawnPos, sprite->spritesheet, 3);
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
    UID uidCard = create_card(depot, pos, spritesheet, animation);

    Deck *deck = (Deck *)depot.AddFacet(uidCard, Facet_Deck);
    deck->count = 3;

    UID uidDeckDrawTrigger = depot.Alloc();
    Trigger *deckDrawTrigger = (Trigger *)depot.AddFacet(uidDeckDrawTrigger, Facet_Trigger);
    deckDrawTrigger->trigger = MsgType_Card_Notify_DragBegin;
    deckDrawTrigger->message.uid = uidCard;
    deckDrawTrigger->callback = deck_draw;
    deckDrawTrigger->userData = (void *)(size_t)uidCard;

    TriggerList *triggerList = (TriggerList *)depot.AddFacet(uidCard, Facet_TriggerList, 0, false);
    triggerList->triggers.insert(uidDeckDrawTrigger);

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
        UID uidSheetCards = load_bitmap(depot, "texture/cards.bmp");
        Spritesheet *sheetCards = (Spritesheet *)depot.AddFacet(uidSheetCards, Facet_Spritesheet);
        sheetCards->cells = 10;
        sheetCards->cellSize = { 100, 150 };
        for (int i = 0; i < sheetCards->cells; i++) {
            sheetCards->animations.push_back({ i, 1 });
        }

        UID uidLighter = create_card(depot, { 100, 100, 0 }, uidSheetCards, 0);
        UID uidBucket = create_card(depot, { 200, 100, 0 }, uidSheetCards, 1);
        create_deck(depot, { 300, 100, 0 }, uidSheetCards, 2);
        //create_card(depot, { 400, 100, 0 }, uidSheetCards, 3);

        UID uidSheetCampfire = load_bitmap(depot, "texture/campfire.bmp");
        Spritesheet *sheetCampfire = (Spritesheet *)depot.AddFacet(uidSheetCampfire, Facet_Spritesheet);
        sheetCampfire->cells = 9;
        sheetCampfire->cellSize = { 256, 256 };
        sheetCampfire->animations.push_back({ 0, 1 });
        sheetCampfire->animations.push_back({ 1, 8 });

        UID uidCampfire = create_card(depot, { 200, 300, 0 }, uidSheetCampfire, 0);

        void *dragEndUserData = (void *)(size_t)uidCampfire;
        add_animation_update_trigger(depot, uidLighter, MsgType_Card_Notify_DragEnd, uidCampfire, 1, only_if_drag_landed_on_trigger_target, dragEndUserData);
        add_animation_update_trigger(depot, uidBucket, MsgType_Card_Notify_DragEnd, uidCampfire, 0, only_if_drag_landed_on_trigger_target, dragEndUserData);
        add_sound_trigger(depot, uidLighter, MsgType_Card_Notify_DragEnd, "audio/fire_start.wav", true, only_if_drag_landed_on_trigger_target, dragEndUserData);
        add_sound_trigger(depot, uidBucket, MsgType_Card_Notify_DragEnd, "audio/fire_extinguish.wav", true, only_if_drag_landed_on_trigger_target, dragEndUserData);
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
