#pragma once
#include "basic.h"
#include "../facets/card.h"
#include <vector>

enum MsgType {
    // Relays all messages to another UID
    MsgType_Special_RelayAllMessages,

    MsgType_Audio_PlaySoundRequest,
    MsgType_Audio_StopSoundRequest,

    MsgType_Card_DoAction,
    MsgType_Card_SpawnCardRequest,
    MsgType_Card_TryToStack,

    MsgType_Combat_Primary,
    MsgType_Combat_Secondary,
    //MsgType_Combat_Secondary_Press,
    MsgType_Combat_Notify_AttackBegin,
    MsgType_Combat_Notify_DefendBegin,
    MsgType_Combat_Notify_IdleBegin,

    MsgType_Cursor_Notify_DragBegin,
    MsgType_Cursor_Notify_DragUpdate,
    MsgType_Cursor_Notify_DragEnd,
    //MsgType_Cursor_PrimaryPress,    // directly from events
    //MsgType_Cursor_PrimaryRelease,  // directly from events

    MsgType_Effect_OnFireBegin,
    MsgType_Effect_OnFireEnd,

    MsgType_FpsCounter_Notify_Update,

    MsgType_Movement_WalkUp,
    MsgType_Movement_WalkLeft,
    MsgType_Movement_WalkDown,
    MsgType_Movement_WalkRight,
    MsgType_Movement_RunUp,
    MsgType_Movement_RunLeft,
    MsgType_Movement_RunDown,
    MsgType_Movement_RunRight,
    MsgType_Movement_Jump,

    MsgType_Physics_ApplyImpulse,
    MsgType_Physics_Notify_Collide,
    MsgType_Physics_Notify_Position_Change,  // TODO: Use this to update overhead text

    MsgType_Render_FrameBegin,
    MsgType_Render_Quit,
    MsgType_Render_ToggleVsync,
    MsgType_Render_ToggleFullscreen,
    MsgType_Render_DbgSetFontNext,
    MsgType_Render_DbgSetFontPrev,

    MsgType_Sprite_UpdateAnimationRequest,

    MsgType_Text_UpdateText,

    MsgType_Window_Quit,
};

struct Msg_Audio_PlaySoundRequest {
    const char * soundKey {};
    bool         override {};
};

struct Msg_Audio_StopSoundRequest {
    const char * soundKey {};
};

struct Msg_Card_SpawnCardRequest {
    CardType     cardType     {};
    const char * cardProtoKey {};
    vec3         spawnPos     {};
    union {
        struct {
            bool isDeckDraw {};
        } card;
        struct {
            int cardCount {};
        } deck;
    } data {};
};

struct Msg_Combat_Notify_AttackBegin {
    int weapon {};  // or whatever
};

struct Msg_Combat_Notify_DefendBegin {
    int shield {};  // or whatever
};

struct Msg_Combat_Primary {
    int world_x {};  // note: world:screen is 1:1... for now
    int world_y {};
};

struct Msg_Cursor_Notify_DragEnd {
    vec2 dragDelta{};  // delta pos of mouse compared to where drag was started
};

struct Msg_Physics_ApplyImpulse {
    vec2 v{};  // impulse vector (dir + mag)
};

struct Msg_Physics_Notify_Collide {
    UID other{};
    // TODO: manifold? (i.e. depth, normal)
};

struct Msg_Render_FrameBegin {
    double realDtSmooth {};
};

struct Msg_Sprite_UpdateAnimationRequest {
    UID          uidSprite      {};
    const char * spritesheetKey {};
    const char * animationKey   {};
    int          frame          {};
};

struct Msg_FpsCounter_Notify_Update {
    const char *str    {};
    vec2        offset {};
    vec4        color  {};
};

struct Depot;
struct Msg_Text_UpdateText;
typedef void (*Text_UpdateTextCallback)(Depot &depot, UID subject, Msg_Text_UpdateText *data);
struct Msg_Text_UpdateText {
    const char             *str      {};
    vec2                    offset   {};
    vec4                    color    {};
    //Text_UpdateTextCallback callback {};
};

struct Message {
    UID     uid  {};  // primary subject of interest
    MsgType type {};
    union {
        Msg_Audio_PlaySoundRequest audio_playsoundrequest;
        Msg_Audio_StopSoundRequest audio_stopsoundrequest;
        Msg_Card_SpawnCardRequest card_spawncardrequest;
        Msg_Combat_Notify_AttackBegin combat_attackbegin;
        Msg_Combat_Notify_DefendBegin combat_defendbegin;
        Msg_Combat_Primary combat_primary;
        Msg_Cursor_Notify_DragEnd cursor_dragend;
        Msg_FpsCounter_Notify_Update fpscounter_notify_update;
        Msg_Physics_ApplyImpulse physics_applyimpulse;
        Msg_Physics_Notify_Collide physics_notify_collide;
        Msg_Render_FrameBegin render_framebegin;
        Msg_Sprite_UpdateAnimationRequest sprite_updateanimationrequest;
        Msg_Text_UpdateText text_updatetext;
    } data {};

    Message(void) {
        memset(&data, 0, sizeof(data));
    }
};

typedef std::vector<Message> MsgQueue;
