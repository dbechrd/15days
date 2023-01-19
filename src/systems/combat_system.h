#pragma once
#include "../common/basic.h"
#include "../common/draw_list.h"

struct Msg_Combat_Notify_AttackBegin {
    int weapon {};  // or whatever
};
struct Msg_Combat_Notify_DefendBegin {
    int shield {};  // or whatever
};
struct Msg_Combat_Primary {
    int world_x {};  // note: world:screen is 1:1... for now
    int world_y {};
};

struct CombatSystem {
    void Display(Depot &depot, DrawQueue &drawQueue);
};