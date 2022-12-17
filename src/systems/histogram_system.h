#pragma once
#include "../common/basic.h"
#include "../common/draw_list.h"

struct HistogramSystem {
    void Update(double now, Depot &depot);
    void Display(double now, Depot &depot, DrawQueue &drawQueue);

private:
};