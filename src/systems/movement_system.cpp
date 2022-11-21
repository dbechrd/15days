#include "movement_system.h"
#include "../common/force.h"
#include "../facets/depot.h"

void MovementSystem::ProcessCommands(double now, Depot &depot, UID uid,
    const CommandQueue &commandQueue, ForceQueue &forceQueue)
{
    Position *position = (Position *)depot.GetFacet(uid, Facet_Position);
    if (!position) return;

    // TODO: Generate an ApplyForce message for the physics engine
    Msg_ApplyForce msg{};

    const float vel = 5.0f;

    for (const CommandType &command : commandQueue) {
        switch (command) {
            case Command_MoveUp:
            {
                msg.force.y -= vel;
                break;
            }
            case Command_MoveLeft: {
                msg.force.x -= vel;
                break;
            }
            case Command_MoveDown: {
                msg.force.y += vel;
                break;
            }
            case Command_MoveRight: {
                msg.force.x += vel;
                break;
            }
            default: break;
        }
    }

    forceQueue.push_back(msg);
}

void MovementSystem::Update(double now, Depot &depot)
{
    for (Combat &combat : depot.combat) {
        if (combat.attackStartedAt) {
            float attackAlpha = (now - combat.attackStartedAt) / combat.attackCooldown;
            if (attackAlpha < 1.0) {
                // TOOD: Something interesting during attack
            } else {
                combat.attackStartedAt = 0;
                combat.attackCooldown = 0;
            }
        }
        if (combat.defendStartedAt) {
            float defendAlpha = (now - combat.defendStartedAt) / combat.defendCooldown;
            if (defendAlpha < 1.0) {
                // TOOD: Something interesting during defend
            } else {
                combat.defendStartedAt = 0;
                combat.defendCooldown = 0;
            }
        }
    }
}