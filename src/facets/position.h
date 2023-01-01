#pragma once
#include "facet.h"

struct Position : public Facet {
    vec3 pos{};
    vec2 size{};  // todo: center/extents or min/max.. should probably be 3D??

    inline float Depth(void)
    {
        return pos.y + size.h;
    }

    inline bool OnGround(void)
    {
        return pos.z == 0;
    }
};