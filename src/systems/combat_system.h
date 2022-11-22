#pragma once
#include "../common/basic.h"
#include "../common/message.h"

struct Depot;

struct CombatSystem {
    void ProcessMessages(double now, Depot &depot, MsgQueue &msgQueue);
    void Update(double now, Depot &depot);
};