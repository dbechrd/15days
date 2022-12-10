#pragma once
#include "../common/basic.h"

struct PhysicsSystem {
    void React(double now, Depot &depot);
    void Behave(double now, Depot &depot, double dt);
};