#pragma once
#include "../common/basic.h"
#include "../common/draw_list.h"

struct CombatSystem {
    void Display(double now, Depot &depot, DrawQueue &drawQueue);
};