#pragma once
#include "../common/error.h"
#include "../common/message.h"
#include "../facets/depot.h"

struct AudioSystem {
    FDOVResult Init(void);

    void React(double now, Depot &depot, MsgQueue &msgQueue);
    void Behave(double now, Depot &depot, double dt);
    //void Display(double now, Depot &depot, DrawQueue &drawQueue);
};