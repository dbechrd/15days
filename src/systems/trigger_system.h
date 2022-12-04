#pragma once
#include "../facets/depot.h"

struct TriggerSystem {
    void React(double now, Depot &depot);
    //void Behave(double now, Depot &depot, double dt);
    //void Display(double now, Depot &depot, DrawQueue &drawQueue);
};