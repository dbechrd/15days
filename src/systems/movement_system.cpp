#include "movement_system.h"
#include "../facets/depot.h"
#include "../common/message.h"
#include "dlb/dlb_types.h"

void MovementSystem::React(double now, Depot &depot, MsgQueue &msgQueue)
{
    size_t size = msgQueue.size();
    for (int i = 0; i < size; i++) {
        Message &msg = msgQueue[i];
        Body *body = (Body *)depot.GetFacet(msg.uid, Facet_Body);
        if (!body) {
            continue;
        }

        switch (msg.type) {
            case MsgType_Input_WalkUp: body->moveBuffer.y -= 1; break;
            case MsgType_Input_WalkLeft: body->moveBuffer.x -= 1; break;
            case MsgType_Input_WalkDown: body->moveBuffer.y += 1; break;
            case MsgType_Input_WalkRight: body->moveBuffer.x += 1; break;
            case MsgType_Input_RunUp: body->moveBuffer.y -= 1; body->runBuffer = true; break;
            case MsgType_Input_RunLeft: body->moveBuffer.x -= 1; body->runBuffer = true; break;
            case MsgType_Input_RunDown: body->moveBuffer.y += 1; body->runBuffer = true; break;
            case MsgType_Input_RunRight: body->moveBuffer.x += 1; body->runBuffer = true; break;
            case MsgType_Input_Jump: body->jumpBuffer = true; break;
            default: break;
        }
    }
}

void MovementSystem::Behave(double now, Depot &depot, double dt)
{

}