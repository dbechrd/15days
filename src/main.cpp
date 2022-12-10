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
    char *key = (char *)depot.resourceArena.Alloc(keyLen);
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
    const char *soundFile, bool override = true)
{
    UID uidAudioBuffer = load_sound(depot, soundFile);

    UID uidTrigger = depot.Alloc();
    printf("%u: trigger on %u to play sound %u\n", uidTrigger, subject, uidAudioBuffer);
    Trigger *trigger = (Trigger *)depot.AddFacet(uidTrigger, Facet_Trigger);
    trigger->trigger = msgType;
    trigger->message.uid = uidAudioBuffer;
    trigger->message.type = MsgType_Audio_PlaySound;
    trigger->message.data.audio_playsound.override = override;

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

UID create_global_keymap(Depot &depot)
{
    // TODO: This is maybe "Menu" or something?
    UID uid = depot.Alloc();
    printf("%u: global keymap\n", uid);

    Keymap *keymap = (Keymap *)depot.AddFacet(uid, Facet_Keymap);
    keymap->hotkeys.emplace_back(HotkeyMod_None, FDOV_SCANCODE_QUIT, 0, 0, Hotkey_Press, MsgType_Render_Quit);
    keymap->hotkeys.emplace_back(HotkeyMod_None, SDL_SCANCODE_ESCAPE, 0, 0, Hotkey_Press, MsgType_Render_Quit);
    keymap->hotkeys.emplace_back(HotkeyMod_None, SDL_SCANCODE_V, 0, 0, Hotkey_Press, MsgType_Render_ToggleVsync);

    keymap->hotkeys.emplace_back(HotkeyMod_Ctrl, FDOV_SCANCODE_MOUSE_LEFT, 0, 0, Hotkey_Press, MsgType_Global_PrimaryPress);
    keymap->hotkeys.emplace_back(HotkeyMod_Ctrl, FDOV_SCANCODE_MOUSE_LEFT, 0, 0, Hotkey_Release | Hotkey_Handled, MsgType_Global_PrimaryRelease);

    Cursor *cursor = (Cursor *)depot.AddFacet(uid, Facet_Cursor);
    UNUSED(cursor);

    return uid;
}

UID create_narrator(Depot &depot, UID subject, UID fontFancy);

UID create_player(Depot &depot, UID fontFixed, UID fontFancy)
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
    debugText->font = fontFixed;
    debugText->str = 0;
    debugText->align = TextAlign_VBottom_HCenter;
    debugText->color = C255(COLOR_WHITE);

    add_sound_trigger(depot, uidPlayer, MsgType_Combat_Notify_AttackBegin, "audio/primary.wav");
    add_sound_trigger(depot, uidPlayer, MsgType_Combat_Notify_DefendBegin, "audio/secondary.wav", false);
    add_sound_trigger(depot, uidPlayer, MsgType_Card_DragBegin, "audio/player_drag_begin.wav");
    add_sound_trigger(depot, uidPlayer, MsgType_Card_DragEnd, "audio/player_drag_end.wav");

    create_narrator(depot, uidPlayer, fontFancy);

    return uidPlayer;
}

UID create_narrator(Depot &depot, UID subject, UID fontFancy)
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
    text->font = fontFancy;
    text->str = "15 Days";
    text->align = TextAlign_VBottom_HCenter;
    text->color = C255(COLOR_WHITE);
    text->dirty = true;

    add_sound_trigger(depot, uidNarrator, MsgType_Card_DragBegin, "audio/narrator_drag_begin.wav");
    add_sound_trigger(depot, uidNarrator, MsgType_Card_DragEnd, "audio/narrator_drag_end.wav");

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

UID create_fps_counter(Depot &depot, UID font)
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
    text->font = font;
    text->str = "00 fps (00.00 ms)";
    text->align = TextAlign_VTop_HLeft;
    text->color = C255(COLOR_WHITE);

    add_sound_trigger(depot, uidFpsCounter, MsgType_Card_DragBegin, "audio/drag_begin.wav");
    add_sound_trigger(depot, uidFpsCounter, MsgType_Card_DragEnd, "audio/drag_end.wav");

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

UID create_card(Depot &depot, vec3 pos, UID font, UID spritesheet, int animation)
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
    debugText->font = font;
    debugText->str = 0;
    debugText->align = TextAlign_VBottom_HCenter;
    debugText->color = C255(COLOR_WHITE);

    add_sound_trigger(depot, uidCard, MsgType_Card_DragBegin, "audio/drag_begin.wav");
    add_sound_trigger(depot, uidCard, MsgType_Card_DragEnd, "audio/drag_end.wav");

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
    UID uidFontFixed = load_font(depot, "font/Hack-Bold.ttf", 16);
    UID uidFontFancy = load_font(depot, "font/KarminaBold.otf", 64);

    // Create an entity to hold the global keymap (the plan is to have a global
    // keymap per gamestate eventually)
    create_global_keymap(depot);

    // Create player/narrator
    create_player(depot, uidFontFixed, uidFontFancy);
    create_fps_counter(depot, uidFontFixed);

    {
        UID uidSpritesheetCards = load_bitmap(depot, "texture/cards.bmp");
        Spritesheet *spritesheet = (Spritesheet *)depot.AddFacet(uidSpritesheetCards, Facet_Spritesheet);
        spritesheet->cells = 10;
        spritesheet->cellSize = { 100, 150 };
        for (int i = 0; i < spritesheet->cells; i++) {
            spritesheet->animations.push_back({ i, 1 });
        }

        create_card(depot, { 100, 100, 0 }, uidFontFixed, uidSpritesheetCards, 0);
        create_card(depot, { 200, 100, 0 }, uidFontFixed, uidSpritesheetCards, 1);
    }
    {
        UID uidSpritesheetCampfire = load_bitmap(depot, "texture/campfire.bmp");
        Spritesheet *spritesheet = (Spritesheet *)depot.AddFacet(uidSpritesheetCampfire, Facet_Spritesheet);
        spritesheet->cells = 8;
        spritesheet->cellSize = { 256, 256 };
        spritesheet->animations.push_back({ 0, 8 });

        create_card(depot, { 200, 300, 0 }, uidFontFixed, uidSpritesheetCampfire, 0);
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
