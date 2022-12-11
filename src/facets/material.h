#pragma once
#include "facet.h"

enum MaterialFlags {
    MaterialFlag_Flammable,
};
struct MaterialProto : public Facet {
    std::bitset<32> flags {};  // material flags
};

enum MaterialState {
    MaterialState_OnFire,
};
struct Material : public Facet {
    UID             materialProto {};
    std::bitset<32> state         {};  // current state of each flag
};