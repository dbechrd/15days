#pragma once
#include "../common/basic.h"
#include "facet.h"
#include "material.h"

struct CardProto : public Facet {
    UID          effectList    {};
    UID          materialProto {};
    UID          spritesheet   {};
    const char * animation     {};
};

struct Card : public Facet {
    UID    stackParent  {};
    UID    stackChild   {};
    UID    cardProto    {};
    double noClickUntil {};  // timestamp of when next click is allowed on card
};
