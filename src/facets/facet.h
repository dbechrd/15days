#pragma once
#include "../common/basic.h"

enum FacetType {
    // Resources
    Facet_Animation,
    Facet_CardProto,
    Facet_Font,
    Facet_MaterialProto,
    Facet_Sound,
    Facet_Spritesheet,
    Facet_Texture,
    // Components
    Facet_Attach,
    Facet_Body,
    Facet_Card,
    Facet_CardStack,
    Facet_Combat,
    Facet_Cursor,
    Facet_Deck,
    Facet_EffectList,
    Facet_FpsCounter,
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