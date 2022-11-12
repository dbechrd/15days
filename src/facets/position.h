#pragma once
#include "facet.h"
#include "../common/basic.h"

struct Position : public Facet {
    Vec3 pos{};

    inline bool OnGround(void)
    {
        return pos.z == 0;
    }
};