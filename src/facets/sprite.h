#pragma once
#include "../common/basic.h"
#include "facet.h"

struct Sprite : public Facet {
    Vec2  size        {};
    Color color       {};
    Color attackColor {};
    //float scale {};
};