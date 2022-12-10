#pragma once
#include "facet.h"
#include "../common/basic.h"
#include "../common/message.h"
#include <unordered_set>

enum TriggerType {
    Trigger_Collide
};

struct Trigger : public Facet {
    MsgType trigger {};  // when to trigger
    Message message {};  // what to do
};

struct TriggerList : public Facet {
    std::unordered_set<UID> triggers{};
};