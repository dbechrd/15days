#pragma once
#include "../common/basic.h"
#include "../common/draw_list.h"
#include "../facets/map.h"

struct MapSystem {
public:

    void Init(Depot &depot);
    void Display(Depot &depot, DrawQueue &drawQueue);

private:
    //UID uidEmptyMapSlotSprite{};

    void DrawMapSlot(Depot &depot, DrawQueue &drawQueue, MapSlot &mapSlot, vec3 drawPos);
};
