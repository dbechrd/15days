#pragma once
#include "facet.h"
#include "../common/basic.h"
#include "../common/message.h"
#include <unordered_set>

enum TriggerType {
    Trigger_Collide
};

struct Trigger;
typedef void (*TriggerCallback)(Depot &depot, const Message &msg,
    const Trigger &trigger, void *userData);

struct Trigger : public Facet {
    MsgType          trigger  {};  // when to trigger
    Message          message  {};  // new message template
    TriggerCallback  callback {};  // callback for message customization
    void            *userData {};  // forwarded to callback
};

struct TriggerList : public Facet {
    std::unordered_set<UID> triggers{};
};