#include "movement_system.h"
#include "../facets/depot.h"
#include "../common/message.h"

void MovementSystem::ProcessMessages(double now, Depot &depot, MsgQueue &msgQueue)
{
    const float vel = 3.0f;

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
                body->acceleration.y -= vel;
                break;
            }
            case MsgType_Input_Left: {
                body->acceleration.x -= vel;
                break;
            }
            case MsgType_Input_Down: {
                body->acceleration.y += vel;
                break;
            }
            case MsgType_Input_Right: {
                body->acceleration.x += vel;
                break;
            }
            default: break;
        }
    }
}
