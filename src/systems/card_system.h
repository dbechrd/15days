#pragma once
#include "../common/basic.h"
#include "../common/collision.h"
#include "../common/draw_list.h"

struct CardSystem {
    void UpdateStacks(Depot &depot, const CollisionList &collisionList);
    void UpdateCards(Depot &depot);
    void Display(double now, Depot &depot, DrawQueue &drawQueue);
};
