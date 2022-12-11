#pragma once
#include "basic.h"

struct Collision {
    UID  uidA  {};
    UID  uidB  {};
    rect bboxA {};
    rect bboxB {};
};

typedef std::vector<Collision> CollisionList;