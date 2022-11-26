#pragma once
#include "../common/draw_list.h"
#include "../common/message.h"
#include "../facets/depot.h"
#include "../facets/sprite.h"

struct SpriteSystem {
    // TODO: I feel like someone else maybe should own this, but idk yet
    static void InitSprite(Sprite &sprite);

    void React(double now, Depot &depot, MsgQueue &msgQueue);
    void Behave(double now, Depot &depot, double dt);
    void Display(double now, Depot &depot, DrawQueue &drawQueue);
};