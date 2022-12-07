#pragma once
#include "../common/basic.h"

struct FpsCounterSystem {
    //void Init(Depot &depot);
    void React(double now, Depot &depot);
    void Behave(double now, Depot &depot, double dt);
};