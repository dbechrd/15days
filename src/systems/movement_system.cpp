#include "movement_system.h"
#include "../facets/depot.h"
#include "../common/message.h"

void MovementSystem::ProcessMessages(double now, Depot &depot, MsgQueue &msgQueue)
{
    const float vel = 5.0f;

    size_t size = msgQueue.size();
    for (int i = 0; i < size; i++) {
        Message &msg = msgQueue[i];
        Position *position = (Position *)depot.GetFacet(msg.uid, Facet_Position);
        if (!position) {
            continue;
        }

        // TODO: "MessageFactory"
        Message impulseMsg{};
        impulseMsg.type = MsgType_Movement_Impulse;
        Msg_Movement_Impulse &impulse = impulseMsg.data.movement_impulse;

        switch (msg.type) {
            case MsgType_Input_Up:
            {
                impulseMsg.uid = msg.uid;
                impulse.v.y -= vel;
                break;
            }
            case MsgType_Input_Left: {
                impulseMsg.uid = msg.uid;
                impulse.v.x -= vel;
                break;
            }
            case MsgType_Input_Down: {
                impulseMsg.uid = msg.uid;
                impulse.v.y += vel;
                break;
            }
            case MsgType_Input_Right: {
                impulseMsg.uid = msg.uid;
                impulse.v.x += vel;
                break;
            }
            default: break;
        }

        // TODO: Modifying vector while iterating it won't work like this
        if (impulseMsg.uid) {
            msgQueue.push_back(impulseMsg);
        }
    }
}

void MovementSystem::Update(double now, Depot &depot)
{
    for (Position &position : depot.position) {

    }
}