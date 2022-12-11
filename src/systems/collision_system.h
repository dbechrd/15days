#pragma once
#include "../common/basic.h"
#include "../common/collision.h"

struct CollisionSystem {
    void DetectCollisions(Depot &depot, CollisionList &collisionList);
};
