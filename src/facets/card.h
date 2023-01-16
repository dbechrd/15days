#pragma once
#include "../common/basic.h"
#include "facet.h"
#include "material.h"

struct Card : public Facet {
    UID          stackParent  {};
    UID          stackChild   {};
    const char * cardProto    {};
    double       noClickUntil {};  // timestamp of when next click is allowed on card
};
