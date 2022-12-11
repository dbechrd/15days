#pragma once
#include "facet.h"

struct Body : public Facet {
    float friction     {};
    float drag         {};
    float restitution  {};
    float jumpImpulse  {};
    float speed        {};
    float runMult      {};
    float gravity      {};
    float invMass      {};

    vec3  velocity     {};
    vec2  moveBuffer   {};
    bool  runBuffer    {};
    bool  jumpBuffer   {};

    inline bool Moving(void)
    {
        return !v3_iszero(&velocity);
    }
};