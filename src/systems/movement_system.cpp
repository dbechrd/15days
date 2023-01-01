#include "movement_system.h"
#include "../facets/depot.h"

void MovementSystem::React(Depot &depot)
{
    size_t size = depot.msgQueue.size();
    for (int i = 0; i < size; i++) {
        Message msg = depot.msgQueue[i];
        Body *body = (Body *)depot.GetFacet(msg.uid, Facet_Body);
        if (!body) {
            continue;
        }

        // TODO: Instead of moveBuffer, why not have impulseBuffer and just
        // enqueue a bunch of MsgType_Physics_ApplyImpulse messages here that
        // will resolve themselves in PhysicsSystem later?
        switch (msg.type) {
            case MsgType_Movement_WalkUp: body->moveBuffer.y -= 1; break;
            case MsgType_Movement_WalkLeft: body->moveBuffer.x -= 1; break;
            case MsgType_Movement_WalkDown: body->moveBuffer.y += 1; break;
            case MsgType_Movement_WalkRight: body->moveBuffer.x += 1; break;
            case MsgType_Movement_RunUp: body->moveBuffer.y -= 1; body->runBuffer = true; break;
            case MsgType_Movement_RunLeft: body->moveBuffer.x -= 1; body->runBuffer = true; break;
            case MsgType_Movement_RunDown: body->moveBuffer.y += 1; body->runBuffer = true; break;
            case MsgType_Movement_RunRight: body->moveBuffer.x += 1; body->runBuffer = true; break;
            case MsgType_Movement_Jump: body->jumpBuffer = true; break;
            default: break;
        }
    }
}