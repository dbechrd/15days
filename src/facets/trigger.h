#pragma once
#include "facet.h"
#include "../common/basic.h"
#include "../common/message.h"
#include <vector>

struct Msg_Trigger {

};

enum TriggerType {
    Trigger_Collide
};

struct Trigger : public Facet {
    MsgType trigger {};  // when to trigger
    Message message {};  // what to do
};

struct TriggerList : public Facet {
    std::vector<UID> triggers{};
};