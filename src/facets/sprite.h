#pragma once
#include "facet.h"

struct Sprite : public Facet {
    UID  spritesheet {};
    int  animation   {};
    int  frame       {};
    vec2 size        {};
    vec4 color       {};
    vec4 attackColor {};
    vec4 defendColor {};
    //float scale {};
};

struct Animation {
    int start {};
    int count {};
};

struct Spritesheet : public Facet {
    vec2 cellSize {};
    int  cells    {};
    std::vector<Animation> animations {};
};
