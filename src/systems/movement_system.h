#pragma once
#include "../common/basic.h"
#include "../common/message.h"

struct Depot;

struct MovementSystem {
    void ProcessMessages(double now, Depot &depot, MsgQueue &msgQueue);
};