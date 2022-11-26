#pragma once
#include "../common/basic.h"
#include "../common/message.h"
#include "../facets/depot.h"

struct PhysicsSystem {
    void React(double now, Depot &depot, MsgQueue &msgQueue);
    void Behave(double now, Depot &depot, double dt);
    //void Display(double now, Depot &depot, DrawQueue &drawQueue);
};