#pragma once
#include "basic.h"

struct DrawCommand {
    UID          uid     {};  // uid of entity that generated this draw command
    vec4         color   {};
    rect         dstRect {};
    rect         srcRect {};
    SDL_Texture *texture {};  // if present, also applies a texture
    float        depth   {};  // lower = draw first
    bool         outline {};  // show border if true

    bool operator<(const DrawCommand &right) const
    {
        return depth < right.depth;
    }
};

//struct DrawCommandComparator {
//    bool operator()(const DrawCommand &l, const DrawCommand &r)
//    {
//        return l.depth > r.depth;
//    }
//};

//typedef std::priority_queue<DrawCommand> SortedDrawQueue;
typedef std::vector<DrawCommand> DrawQueue;