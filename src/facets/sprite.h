#pragma once
#include "../common/basic.h"
#include "facet.h"

struct Sprite : public Facet {
    vec2 size        {};
    vec4 color       {};
    vec4 attackColor {};
    vec4 defendColor {};
    //float scale {};
};