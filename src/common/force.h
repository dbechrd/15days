#pragma once
#include "../common/basic.h"
#include <vector>

struct Msg_ApplyForce {
    vec2 force {};
};

typedef std::vector<Msg_ApplyForce> ForceQueue;