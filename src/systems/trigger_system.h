#pragma once
#include "../common/basic.h"
#include "../common/message.h"
#include "../facets/trigger.h"

struct TriggerSystem {
    void Trigger_Text_UpdateText(Depot &depot, UID src, MsgType msgType, UID dst, const char *str, vec4 color);
    void Trigger_Special_RelayAllMessages(Depot &depot, UID src, UID dst, TriggerCallback callback);

    void React(Depot &depot);

private:
    void CheckTriggers(Depot &depot, TriggerList &triggerList, Message &msg);
};