#pragma once
#include "../common/basic.h"
#include "../common/draw_list.h"
#include "../facets/sprite.h"

struct SpriteSystem {
    void React(double now, Depot &depot);
    void Behave(double now, Depot &depot, double dt);
    void Display(double now, Depot &depot, DrawQueue &drawQueue);

    // TODO: I feel like someone else maybe should own this, but idk yet
    static void InitSprite(Sprite &sprite);
};