#pragma once
#include "facet.h"
#include "../../include/dlb/dlb_math.h"

// Location of mouse cursor in screen coords (and world coords if relevant)
struct Cursor : public Facet {
    double leftButtonDownAt      {};
    bool   leftButtonQuickClick  {};
    double rightButtonDownAt     {};
    double rightButtonQuickClick {};
    UID    uidDragSubject        {};  // UID of entity being dragged by this cursor, if any
    vec2   dragOffset            {};  // offset of where entity was clicked on dragBegin
    double quickClickMaxDt       {};  // max dt in seconds for click to be considered "quick"
    //// TODO: CameraSystem should probably populate this?
    //bool  worldActive {};  // if false, world position is undefined
    //vec2  posWorld       {};  // position of cursor in world coordinates
};
