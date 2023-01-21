#pragma once
#include "../common/basic.h"
#include "facet.h"

#define MAP_MAX_W 6
#define MAP_MAX_H 4

struct MapSlot {
    const char *mapRoomKey {};
    UID card {};
};

struct Map : public Facet {
    MapSlot slots[MAP_MAX_H][MAP_MAX_W]{};
};
