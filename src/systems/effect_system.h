#pragma once
#include "../common/basic.h"
#include "../common/collision.h"

struct EffectSystem {
    void ApplyDragFx(Depot &depot, const CollisionList &collisionList);
    void ApplyFx_AnyToAny(Depot &depot, const CollisionList &collisionList);
};
