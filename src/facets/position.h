#pragma once
#include "facet.h"

struct Position : public Facet {
    vec3 pos{};

    inline bool OnGround(void)
    {
        return pos.z == 0;
    }
};