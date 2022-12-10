#pragma once
#include "../common/basic.h"
#include "../common/draw_list.h"
#include "../facets/sprite.h"

struct SpriteSystem {
    // TODO: I feel like someone else maybe should own this, but idk yet
    static void InitSprite(Sprite &sprite, vec2 size, vec4 color);

    void React(double now, Depot &depot);
    void Behave(double now, Depot &depot, double dt);
    void Display(double now, Depot &depot, DrawQueue &drawQueue);

private:
    const double fixedAnimDt = 1.0 / 20.0;
    double lastAnimAt = 0;
};