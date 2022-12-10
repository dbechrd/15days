#pragma once
#include "../common/basic.h"
#include "../common/draw_list.h"

struct TextSystem {
    void React(double now, Depot &depot);
    void Display(double now, Depot &depot, DrawQueue &drawQueue);
};