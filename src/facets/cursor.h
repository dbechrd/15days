#pragma once
#include "facet.h"
#include "../../include/dlb/dlb_math.h"

// Location of mouse cursor in screen coords (and world coords if relevant)
struct Cursor : public Facet {
    bool  leftButton     {};
    bool  rightButton    {};
    UID   uidDragSubject {};  // UID of entity being dragged by this cursor, if any
    vec2  dragOffset     {};  // offset of where entity was clicked on dragBegin
    //// TODO: CameraSystem should probably populate this?
    //bool  worldActive {};  // if false, world position is undefined
    //vec2  posWorld       {};  // position of cursor in world coordinates
};
