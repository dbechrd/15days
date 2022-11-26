#include "physics_system.h"
#include "../facets/depot.h"

void PhysicsSystem::React(double now, Depot &depot, MsgQueue &msgQueue)
{

}

void PhysicsSystem::Behave(double now, Depot &depot, double dt)
{
    for (Body &body : depot.body) {
        Position *position = (Position *)depot.GetFacet(body.uid, Facet_Position);
        if (!position) {
            printf("Warn: Physics body has no position: %u\n", body.uid);
            continue;
        }

        //body.moveBuffer.x = CLAMP(body.moveBuffer.x, -1, 1);
        //body.moveBuffer.y = CLAMP(body.moveBuffer.y, -1, 1);
        //body.moveBuffer.z = CLAMP(body.moveBuffer.z, -1, 1);
        v3_normalize(&body.moveBuffer);
        v3_scalef(&body.moveBuffer, 3.0f);  // TODO: body->speed ?

        vec3 acc{};
        v3_add(&acc, &body.gravity);
        v3_add(&acc, &body.moveBuffer);

        const float fK = 1.0f - body.friction;
        v3_add(&body.velocity, &acc);
        v3_scalef(&body.velocity, fK);
        v3_add(&position->pos, &body.velocity);

        body.moveBuffer = VEC3_ZERO;
    }
}