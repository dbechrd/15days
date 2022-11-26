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
            case MsgType_Input_Up:
            {
                body->moveBuffer.y -= 1;
                break;
            }
            case MsgType_Input_Left: {
                body->moveBuffer.x -= 1;
                break;
            }
            case MsgType_Input_Down: {
                body->moveBuffer.y += 1;
                break;
            }
            case MsgType_Input_Right: {
                body->moveBuffer.x += 1;
                break;
            }
            default: break;
        }
    }
}

void MovementSystem::Behave(double now, Depot &depot, double dt)
{

}