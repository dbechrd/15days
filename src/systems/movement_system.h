#pragma once
#include "../common/basic.h"
#include "../common/command.h"
#include "../common/force.h"

struct Depot;

struct MovementSystem {
    void ProcessCommands(double now, Depot &depot, UID uid,
        const CommandQueue &commandQueue, ForceQueue &forceQueue);
    void Update(double now, Depot &depot);
};