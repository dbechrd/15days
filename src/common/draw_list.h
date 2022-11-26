#pragma once
#include "basic.h"
#include <vector>
#include <queue>

struct SDL_Texture;

struct DrawCommand {
    vec4         color {};
    rect         rect  {};
    SDL_Texture *tex   {};  // if null, draws colored rect
    float        depth {};  // lower = draw first

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