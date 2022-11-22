#pragma once
#include "facet.h"
#include "../common/basic.h"
#include "../common/message.h"

struct Msg_Trigger {

};

enum TriggerType {
    Trigger_Collide
};

struct Trigger : public Facet {
    UID         subject {};  // who to watch
    UID         target  {};  // who to affect
    TriggerType type    {};  // when to trigger
    MsgType     msgType {};  // what to do
};