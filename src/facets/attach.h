#pragma once
#include "facet.h"

// TODO: If these don't share sensible names across different entities, we
// should probably just use a dynamic list of string -> vec2
enum AttachPoint {
    AttachPoint_Gut,
    AttachPoint_Count
};

struct Attach : public Facet {
    vec2 points[AttachPoint_Count]{};
};