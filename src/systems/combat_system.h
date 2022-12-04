#pragma once
#include "../common/basic.h"
#include "../facets/depot.h"

struct CombatSystem {
    void React(double now, Depot &depot);
    void Behave(double now, Depot &depot, double dt);
    //void Display(double now, Depot &depot, DrawQueue &drawQueue);
};