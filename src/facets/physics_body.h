#pragma once
#include "facet.h"
#include "../common/basic.h"

struct PhysicsBody : public Facet {
    float friction {};
    float gravity  {};
    Vec3  velocity {};

    inline bool Moving(void)
    {
        return !velocity.IsZero();
    }
};