#pragma once
#include "../common/basic.h"

enum FacetType {
    // Resources
    Facet_Font,
    Facet_Sound,
    Facet_Texture,
    // Components
    Facet_Attach,
    Facet_Body,
    Facet_Combat,
    Facet_Cursor,
    Facet_FpsCounter,
    Facet_Keymap,
    Facet_Text,
    Facet_Position,
    Facet_Sprite,
    Facet_Trigger,
    Facet_TriggerList,
    Facet_Count,
};

struct Facet {
    UID         uid  {};
    FacetType   type {};
    const char *name {};
};