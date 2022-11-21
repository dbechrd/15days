#pragma once
#include "basic.h"
#include <vector>

struct DrawCommand {
    vec4 color {};
    rect  rect  {};
};

typedef std::vector<DrawCommand> DrawList;