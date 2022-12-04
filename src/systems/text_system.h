#pragma once
#include "../common/draw_list.h"
#include "../facets/depot.h"

struct TextSystem {
    void React(double now, Depot &depot);
    void Behave(double now, Depot &depot, double dt);
    void Display(double now, Depot &depot, DrawQueue &drawQueue);
};