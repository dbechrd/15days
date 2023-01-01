#pragma once
#include "../common/basic.h"
#include "../common/message.h"
#include "../facets/trigger.h"

struct TriggerSystem {
    void React(Depot &depot);

private:
    void CheckTriggers(Depot &depot, TriggerList &triggerList, Message &msg);
};