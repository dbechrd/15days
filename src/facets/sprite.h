#pragma once
#include "facet.h"
#include "../common/basic.h"

struct Sprite : public Facet {
    UID  texture     {};
    vec2 size        {};
    vec4 color       {};
    vec4 attackColor {};
    vec4 defendColor {};
    //float scale {};
};