#pragma once
#include "facet.h"
#include "../common/basic.h"

// TODO: If these don't share sensible names across different entities, we
// should probably just use a dynamic list of string -> Vec2
enum AttachPoint {
    AttachPoint_Gut,
    AttachPoint_Count
};

struct Attach : public Facet {
    Vec2 points[AttachPoint_Count]{};
};