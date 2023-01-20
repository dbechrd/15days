#pragma once
#include "basic.h"

enum MsgType {
    // Relays all messages to another UID
    MsgType_Special_RelayAllMessages,

    MsgType_Card_TryToStack,

    MsgType_Combat_Primary,
    MsgType_Combat_Secondary,

    MsgType_Cursor_Notify_DragBegin,
    MsgType_Cursor_Notify_DragUpdate,
    MsgType_Cursor_Notify_DragEnd,
    //MsgType_Cursor_PrimaryPress,    // directly from events
    //MsgType_Cursor_PrimaryRelease,  // directly from events

    //MsgType_Effect_OnFireBegin,
    //MsgType_Effect_OnFireEnd,

    MsgType_Material_StateChange,

    MsgType_Movement_WalkUp,
    MsgType_Movement_WalkLeft,
    MsgType_Movement_WalkDown,
    MsgType_Movement_WalkRight,
    MsgType_Movement_RunUp,
    MsgType_Movement_RunLeft,
    MsgType_Movement_RunDown,
    MsgType_Movement_RunRight,
    MsgType_Movement_Jump,

    //MsgType_Physics_ApplyImpulse,
    //MsgType_Physics_Notify_Collide,
    //MsgType_Physics_Notify_Position_Change,  // TODO: Use this to update overhead text

    MsgType_Render_FrameBegin,
    MsgType_Render_Quit,
    MsgType_Render_ToggleVsync,
    MsgType_Render_ToggleFullscreen,
    MsgType_Render_DbgSetFontNext,
    MsgType_Render_DbgSetFontPrev,

    MsgType_Window_Quit,
};

struct Msg_Cursor_Notify_DragEvent {
    vec2 startPos      {};  // cursor position when drag event started
    vec2 currentPos    {};  // current cursor position
    vec2 dragOffset    {};  // currentPos - startPos, for convenience
    vec2 subjectOffset {};  // initial cursor offset from position of drag subject
};

struct Msg_Material_StateChange {
    ResourceDB::MaterialStates oldStates   {};
    ResourceDB::MaterialStates newStates   {};
    ResourceDB::MaterialStates deltaStates {};
};

struct Message {
    UID     uid  {};  // primary subject of interest
    MsgType type {};
    union {
        Msg_Cursor_Notify_DragEvent cursor_dragevent;
        Msg_Material_StateChange material_statechange;
        //Msg_Render_FrameBegin render_framebegin;
    } data {};

    Message(void) {
        memset(&data, 0, sizeof(data));
    }
};

typedef std::vector<Message> MsgQueue;
