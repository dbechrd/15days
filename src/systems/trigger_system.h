#pragma once
#include "../common/basic.h"
#include "../common/message.h"
#include "../facets/trigger.h"

struct TriggerSystem {
    void React(double now, Depot &depot);
    //void Behave(double now, Depot &depot, double dt);
    //void Display(double now, Depot &depot, DrawQueue &drawQueue);

private:
    void CheckTriggers(Depot &depot, TriggerList &triggerList, Message &msg);
};