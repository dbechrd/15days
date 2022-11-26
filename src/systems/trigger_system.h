#pragma once
#include "../common/message.h"
#include "../facets/depot.h"

struct TriggerSystem {
    void React(double now, Depot &depot, MsgQueue &msgQueue);
    //void Behave(double now, Depot &depot, double dt);
    //void Display(double now, Depot &depot, DrawQueue &drawQueue);
};