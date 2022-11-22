#pragma once
#include "../common/basic.h"

enum FacetType {
    Facet_Attach,
    Facet_Body,
    Facet_Combat,
    Facet_Keymap,
    Facet_Position,
    Facet_Sprite,
    Facet_Trigger,
    Facet_Count,
};

struct Facet {
    UID       uid  {};
    FacetType type {};
};