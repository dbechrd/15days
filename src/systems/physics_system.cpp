#include "physics_system.h"
#include "../facets/depot.h"

void PhysicsSystem::React(double now, Depot &depot)
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

        if (fabsf(body.velocity.z) < MATH_EPSILON &&
            fabsf(position->pos.z) < MATH_EPSILON)
        {
            position->pos.z = 0;
            body.velocity.z = 0;
        }

        //body.moveBuffer.x = CLAMP(body.moveBuffer.x, -1, 1);
        //body.moveBuffer.y = CLAMP(body.moveBuffer.y, -1, 1);
        //body.moveBuffer.z = CLAMP(body.moveBuffer.z, -1, 1);
        v2_normalize(&body.moveBuffer);
        v2_scalef(&body.moveBuffer, body.speed);
        if (body.runBuffer) {
            v2_scalef(&body.moveBuffer, body.runMult);
        }

        // Generate impulse from input buffers
        vec3 impulse{};
        impulse.x += body.moveBuffer.x;
        impulse.y += body.moveBuffer.y;
        impulse.z += body.gravity;
        if (position->pos.z == 0.0f) {
            impulse.z += body.jumpBuffer ? body.jumpImpulse : 0;
        }

        // Apply impulse
        //v3_scalef(&impulse, dt);
        v3_add(&body.velocity, &impulse);

        // Calculate friction from new velocity
        vec3 friction{};
        if (position->OnGround() && body.velocity.z < 0) {
            if (impulse.x > 0.0f) {
                printf("");
            }
            // Apply ground friction
            vec3 normal{};
            normal.z = -body.velocity.z;
            v3_scalef(&normal, body.friction);
            float fK = v3_length(&normal);
            friction = { -body.velocity.x, -body.velocity.y, 0 };
            v3_scalef(&friction, fK);
        }

        // Apply air drag
        vec3 drag = body.velocity;
        v3_negate(&drag);
        v3_scalef(&drag, body.drag);

        //v3_scalef(&friction, dt);
        //v3_scalef(&drag, dt);
        v3_add(&body.velocity, &friction);
        v3_add(&body.velocity, &drag);

        // Update position
        vec3 vel = body.velocity;
        v3_scalef(&vel, dt);
        v3_add(&position->pos, &vel);

        // Bounce on ground
        if (position->pos.z < 0.0f) {
            // TODO: Find exact intersection t and apply leftover dt to new velocity
            position->pos.z = 0; // *= -1;
            body.velocity.z *= -body.restitution;
        }

        // Update debug text
        size_t maxLen = 1024;
        char *debugText = (char *)depot.frameArena.Alloc(maxLen);
        if (!debugText) {
            printf("ERROR: failed to allocate space on frame arena for physics debug text for entity %u\n", body.uid);
            continue;
        }

        snprintf(debugText, maxLen,
            "drg: %7.2f %7.2f %7.2f\n"
            "frc: %7.2f %7.2f %7.2f\n"
            "vel: %7.2f %7.2f %7.2f",
            drag.x,
            drag.y,
            drag.z,
            friction.x,
            friction.y,
            friction.z,
            body.velocity.x,
            body.velocity.y,
            body.velocity.z
        );

        Message updateBodyDebugText{};
        updateBodyDebugText.uid = body.uid;
        updateBodyDebugText.type = MsgType_Text_Change;
        updateBodyDebugText.data.trigger_text_change.str = debugText;
        updateBodyDebugText.data.trigger_text_change.color = C255(COLOR_WHITE);
        if (sprite) {
            updateBodyDebugText.data.trigger_text_change.offset = {
                sprite->size.w / 2,
                -10
            };
        }
        depot.msgQueue.push_back(updateBodyDebugText);

        // Reset input buffers
        body.moveBuffer = VEC2_ZERO;
        body.runBuffer = false;
        body.jumpBuffer = false;
    }
}