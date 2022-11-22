#pragma once
#include "facet.h"
#include "../common/basic.h"

struct Body : public Facet {
    float friction     {};
    float gravity      {};
    vec3  velocity     {};
    vec3  acceleration {};

    inline bool Moving(void)
    {
        return !v3_iszero(&velocity);
    }
};