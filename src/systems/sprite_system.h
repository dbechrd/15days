#pragma once
#include "../common/draw_list.h"
#include "../facets/sprite.h"

struct Depot;

struct SpriteSystem {
    void InitSprite(Sprite &sprite);
    void Update(double now, Depot &depot);
    void Draw(double now, Depot &depot, DrawList &drawList);
};