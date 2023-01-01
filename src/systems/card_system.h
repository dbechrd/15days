#pragma once
#include "../common/basic.h"
#include "../common/collision.h"
#include "../common/draw_list.h"

struct CardSystem {
    void UpdateCards(Depot &depot);
    void UpdateStacks(Depot &depot, const CollisionList &collisionList);
    void Display(Depot &depot, DrawQueue &drawQueue);
};
