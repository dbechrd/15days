#pragma once
#include "../common/basic.h"
#include "../common/collision.h"

struct CursorSystem {
    void UpdateCursors(Depot &depot);
    void UpdateDragTargets(Depot &depot, const CollisionList &collisionList);
};
