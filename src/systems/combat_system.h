#pragma once
#include "../common/basic.h"
#include "../common/command.h"

struct Depot;

struct CombatSystem {
    void ProcessCommands(double now, Depot &depot, UID uid, const CommandQueue &commandQueue);
    void Update(double now, Depot &depot);
};