#pragma once
#include "facet.h"

enum MaterialState {
    MaterialState_OnFire,
};
struct Material : public Facet {
    const char *    materialProtoKey {};
    std::bitset<32> state            {};  // current state of each flag
};