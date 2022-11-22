#include "physics_system.h"
//#include "../common/message.h"
#include "../facets/depot.h"

//void PhysicsSystem::ProcessMessages(double now, Depot &depot, MsgQueue &msgQueue)
//{
//    size_t size = msgQueue.size();
//    for (int i = 0; i < size; i++) {
//        Message &msg = msgQueue[i];
//        Combat *combat = (Combat *)depot.GetFacet(msg.uid, Facet_Combat);
//        if (!combat) {
//            continue;
//        }
//
//        bool canAttack = !(combat->attackStartedAt || combat->defendStartedAt);
//        bool canDefend = !(combat->attackStartedAt);
//
//        switch (msg.type) {
//            case MsgType_Input_Primary:
//            {
//                if (canAttack) {
//                    combat->attackStartedAt = now;
//                    combat->attackCooldown = 0.1;
//                }
//                break;
//            }
//            case MsgType_Input_Secondary:
//            {
//                if (canDefend) {
//                    combat->defendStartedAt = now;
//                    combat->defendCooldown = 0.6;
//                }
//                break;
//            }
//            default: break;
//        }
//    }
//}

void PhysicsSystem::Update(double now, Depot &depot)
{
    for (Body &body : depot.body) {
        const float fK = 1.0f - body.friction;
        v3_add(&body.velocity, &body.acceleration);
        v3_scalef(&body.velocity, fK);

        Position *position = (Position *)depot.GetFacet(body.uid, Facet_Position);
        if (position) {
            v3_add(&position->pos, &body.velocity);
        } else {
            printf("Warn: Physics body has no position: %u\n", body.uid);
        }

        body.acceleration = VEC3_ZERO;
    }
}