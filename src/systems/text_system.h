#pragma once
#include "../common/basic.h"
#include "../common/draw_list.h"

struct TextSystem {
    struct Font *FindOrLoadFont(Depot &depot, const char *fontKey);

    void React(Depot &depot);
    void Display(Depot &depot, DrawQueue &drawQueue);
};