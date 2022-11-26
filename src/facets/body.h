#pragma once
#include "facet.h"
#include "../common/basic.h"

struct Body : public Facet {
    float friction     {};
    vec3  gravity      {};
    vec3  velocity     {};
    vec3  moveBuffer   {};

    inline bool Moving(void)
    {
        return !v3_iszero(&velocity);
    }
};