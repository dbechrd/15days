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
        Sprite *sprite = (Sprite *)depot.GetFacet(body.uid, Facet_Sprite);
        if (!sprite) {
            printf("Warn: Physics body has no sprite: %u\n", body.uid);
            continue;
        }

        //body.moveBuffer.x = CLAMP(body.moveBuffer.x, -1, 1);
        //body.moveBuffer.y = CLAMP(body.moveBuffer.y, -1, 1);
        //body.moveBuffer.z = CLAMP(body.moveBuffer.z, -1, 1);
        v2_normalize(&body.moveBuffer);
        v2_scalef(&body.moveBuffer, body.speed);
        if (body.runBuffer) {
            v2_scalef(&body.moveBuffer, body.runMult);
        }

        vec3 acc{};
        acc.x += body.moveBuffer.x;
        acc.y += body.moveBuffer.y;
        acc.z += body.gravity;
        if (position->pos.z == 0.0f) {
            acc.z += body.jumpBuffer ? body.jumpImpulse : 0;
        }

        float fK;
        if (position->pos.z == 0.0f) {
            fK = 1.0f - body.friction;
        } else {
            fK = 1.0f - body.drag;
        }
        v3_add(&body.velocity, &acc);
        v3_scalef(&body.velocity, fK);
        v3_add(&position->pos, &body.velocity);

        // TODO: Find exact intersection t and apply leftover dt to new velocity
        if (position->pos.z < 0.0f) {
            position->pos.z = 0.0f;
            body.velocity.z *= -body.restitution;
        }

        body.moveBuffer = VEC2_ZERO;
        body.runBuffer = false;
        body.jumpBuffer = false;
    }
}