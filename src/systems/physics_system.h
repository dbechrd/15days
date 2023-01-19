#pragma once
#include "../common/basic.h"

//struct Msg_Physics_ApplyImpulse {
//    vec2 v {};  // impulse vector (dir + mag)
//};
//struct Msg_Physics_Notify_Collide {
//    UID other {};
//    // TODO: manifold? (i.e. depth, normal)
//};

struct PhysicsSystem {
    void Update(Depot &depot, double dt);
};