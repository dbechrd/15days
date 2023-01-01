#pragma once
#include "../common/basic.h"
#include "../common/draw_list.h"

struct HistogramSystem {
    void Update(Depot &depot);
    void Display(Depot &depot, DrawQueue &drawQueue);

private:
};