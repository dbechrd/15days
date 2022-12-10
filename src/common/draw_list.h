#pragma once
#include "basic.h"
#include <vector>
#include <queue>

struct SDL_Texture;

struct DrawCommand {
    UID   uid     {};  // uid of entity that generated this draw command
    vec4  color   {};
    rect  rect    {};
    UID   texture {};  // if present, also applies a texture
    UID   sprite  {};  // if present, uses current animation frame
    float depth   {};  // lower = draw first

    bool operator<(const DrawCommand &right) const
    {
        return depth > right.depth;
    }
};

//struct DrawCommandComparator {
//    bool operator()(const DrawCommand &l, const DrawCommand &r)
//    {
//        return l.depth > r.depth;
//    }
//};

typedef std::priority_queue<DrawCommand> DrawQueue;