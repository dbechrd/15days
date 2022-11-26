#pragma once
#include "../common/basic.h"

enum FacetType {
    Facet_Attach,
    Facet_Body,
    Facet_Combat,
    Facet_Keymap,
    Facet_Text,
    Facet_Position,
    Facet_Sprite,
    Facet_Trigger,
    Facet_TriggerList,
    Facet_Count,
};

struct Facet {
    UID       uid  {};
    FacetType type {};
};