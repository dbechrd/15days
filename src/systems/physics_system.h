#pragma once
#include "../common/basic.h"

struct Depot;

struct PhysicsSystem {
    void Update(double now, Depot &depot);
};