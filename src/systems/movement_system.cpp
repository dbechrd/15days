#include "movement_system.h"
#include "../facets/depot.h"
#include "../common/message.h"
#include "dlb/dlb_types.h"

void MovementSystem::ProcessMessages(double now, Depot &depot, MsgQueue &msgQueue)
{
    size_t size = msgQueue.size();
    for (int i = 0; i < size; i++) {
        Message &msg = msgQueue[i];
        Body *body = (Body *)depot.GetFacet(msg.uid, Facet_Body);
        if (!body) {
            continue;
        }

        vec3 acc{};
        switch (msg.type) {
            case MsgType_Input_Up:
            {
                acc.y -= 1;
                break;
            }
            case MsgType_Input_Left: {
                acc.x -= 1;
                break;
            }
            case MsgType_Input_Down: {
                acc.y += 1;
                break;
            }
            case MsgType_Input_Right: {
                acc.x += 1;
                break;
            }
            default: break;
        }

        acc.x = CLAMP(acc.x, -1, 1);
        acc.y = CLAMP(acc.y, -1, 1);
        v3_normalize(&acc);
        v3_scalef(&acc, 3.0f);  // TODO: body->speed ?
        v3_add(&body->acceleration, &acc);
    }
}
