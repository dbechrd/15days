#pragma once
#include "../common/basic.h"

enum FacetType {
    // Resources
    Facet_Animation,
    Facet_Font,
    Facet_Sound,
    Facet_Texture,
    // Components
    Facet_Attach,
    Facet_Body,
    Facet_Card,
    Facet_Combat,
    Facet_Cursor,
    Facet_FpsCounter,
    Facet_Histogram,
    Facet_Keymap,
    Facet_Material,
    Facet_Text,
    Facet_Position,
    Facet_Sprite,
    Facet_TriggerList,
    Facet_Count,
};

struct Facet {
    UID         uid    {};
    FacetType   f_type {};
    const char *f_name {};
};