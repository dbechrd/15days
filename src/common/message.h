#pragma once
#include "basic.h"
#include <vector>

enum MsgType {
    MsgType_Audio_PlaySound,

    MsgType_Card_DragBegin,
    MsgType_Card_DragEnd,

    MsgType_Combat_Primary,
    MsgType_Combat_Secondary,
    //MsgType_Combat_Secondary_Press,
    MsgType_Combat_Notify_AttackBegin,
    MsgType_Combat_Notify_DefendBegin,

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

    MsgType_Render_Quit,
    MsgType_Render_ToggleVsync,

    MsgType_Text_UpdateText,

    MsgType_Window_Quit,
};

struct Msg_Audio_PlaySound {
    bool override {};
};

struct Msg_Combat_Primary {
    int world_x {};  // note: world:screen is 1:1... for now
    int world_y {};
};

struct Msg_Combat_Notify_AttackBegin {
    int weapon {};  // or whatever
};

struct Msg_Combat_Notify_DefendBegin {
    int shield {};  // or whatever
};

struct Msg_Physics_ApplyImpulse {
    vec2 v {};  // impulse vector (dir + mag)
};

struct Msg_Physics_Notify_Collide {
    UID other {};
    // TODO: manifold? (i.e. depth, normal)
};

struct Depot;
struct Msg_Text_UpdateText;
typedef void (*Text_UpdateTextCallback)(Depot &depot, UID subject, Msg_Text_UpdateText *data);
struct Msg_Text_UpdateText {
    const char             *str      {};
    vec2                    offset   {};
    vec4                    color    {};
    Text_UpdateTextCallback callback {};
};

struct Message {
    UID     uid  {};  // primary subject of interest
    MsgType type {};
    union {
        Msg_Combat_Notify_AttackBegin combat_attackbegin;
        Msg_Combat_Notify_DefendBegin combat_defendbegin;
        Msg_Physics_ApplyImpulse physics_applyimpulse;
        Msg_Physics_Notify_Collide physics_notify_collide;
        Msg_Audio_PlaySound audio_playsound;
        Msg_Text_UpdateText text_updatetext;
    } data {};

    Message(void) {
        memset(&data, 0, sizeof(data));
    }
};

typedef std::vector<Message> MsgQueue;
