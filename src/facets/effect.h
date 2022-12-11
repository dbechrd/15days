#pragma once
#include "facet.h"
#include "../common/message.h"

enum EffectType {
    Effect_IgniteFlammable,
    Effect_ExtinguishFlammable,
};

// TODO(cleanup): I thought i needed a tagged union.. but maybe not..
struct Effect {
    EffectType type {};
};

struct EffectList : public Facet {
    std::vector<Effect> effects {};
};

// lighter card
// when drag start: open lid/ignite sound (can be in one wav file)
// while dragging: burning sound, fire/smoke particles, fire AoE effect
// when drag end: lighter close sound

// campfire card
// flammable material

// water bucket card
// while dragging: sloshing sound, water particles, water AoE effect
// when drag end: bucket thud on ground sound

// collision system
// if fire AoE effect enters flammable region, ignite the flammable
// if water AoE effect enters flammable region, extinguish the flammable