#include "physics_system.h"
#include "../facets/depot.h"

void PhysicsSystem::Update(double now, Depot &depot)
{
    for (Body &body : depot.body) {
        const float fK = 1.0f - body.friction;
        v3_add(&body.velocity, &body.acceleration);
        v3_scalef(&body.velocity, fK);

        Position *position = (Position *)depot.GetFacet(body.uid, Facet_Position);
        if (position) {
            v3_add(&position->pos, &body.velocity);
        } else {
            printf("Warn: Physics body has no position: %u\n", body.uid);
        }

        body.acceleration = VEC3_ZERO;
    }
}