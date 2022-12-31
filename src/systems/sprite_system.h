#pragma once
#include "../common/basic.h"
#include "../common/draw_list.h"
#include "../facets/sprite.h"

struct SpriteSystem {
    // TODO: I feel like someone else maybe should own this, but idk yet
    static void InitSprite(Depot &depot, Sprite &sprite, vec4 color, UID uidSpritesheet);

    void Update(double now, Depot &depot);
    void Display(double now, Depot &depot, DrawQueue &drawQueue);

private:
    const double fixedAnimDt = 1.0 / 20.0;
    double lastAnimAt = 0;
};