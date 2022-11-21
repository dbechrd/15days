#pragma once
#include "facet.h"
#include "../common/basic.h"

struct PhysicsBody : public Facet {
    float friction {};
    float gravity  {};
    vec3  velocity {};

    inline bool Moving(void)
    {
        return !v3_iszero(&velocity);
    }
};