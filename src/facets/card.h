#pragma once
#include "facet.h"
#include "material.h"

struct CardProto : public Facet {
    UID effectList    {};
    UID materialProto {};
    UID spritesheet   {};
    int animation     {};
};

struct Card : public Facet {
    UID    cardProto    {};
    double noClickUntil {};  // timestamp of when next click is allowed on card
};

struct CardStack : public Facet {
    std::vector<UID> cards {};
};