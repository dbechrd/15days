#pragma once
#include "../common/basic.h"
#include "../common/draw_list.h"

struct TextSystem {
    UID LoadFont(Depot &depot, const char *filename, int ptsize);

    void React(Depot &depot);
    void Display(Depot &depot, DrawQueue &drawQueue);
};