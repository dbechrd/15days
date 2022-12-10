#pragma once
#include "../common/basic.h"

struct PhysicsSystem {
    void Update(double now, Depot &depot, double dt);
};