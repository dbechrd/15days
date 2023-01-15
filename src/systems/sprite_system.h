#pragma once
#include "../common/basic.h"
#include "../common/draw_list.h"
#include "../facets/sprite.h"

struct SpriteSystem {
    // TODO: I feel like someone else maybe should own this, but idk yet
    static void InitSprite(Depot &depot, Sprite &sprite, vec4 color,
        UID uidSpritesheet, const char *animation);

    void React(Depot &depot);
    void Update(Depot &depot);

private:
    const double fixedAnimDt = 1.0 / 20.0;
    double lastAnimAt = 0;
};