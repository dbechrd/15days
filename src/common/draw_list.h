#pragma once
#include "basic.h"
#include <vector>

struct DrawCommand {
    Color color {};
    Rect  rect  {};
};

typedef std::vector<DrawCommand> DrawList;