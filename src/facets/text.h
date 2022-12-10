#pragma once
#include "facet.h"
#include "../common/basic.h"
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"

enum TextAlign {
    TextAlign_VTop_HLeft,
    TextAlign_VBottom_HCenter,
};

struct TextProps {
    // values of relevant properties when cached texture was last generated
    UID  font  {};
    vec4 color {};
    char *str  {};

    void Destroy(void) {
        free(str);
        *this = {};
    }
};

struct Text : public Facet {
    UID         font       {};  // will invalidate cache
    vec4        color      {};  // will invalidate cache
    const char *str        {};  // will invalidate cache
    bool        dirty      {};
    //TextProps   cacheProps {};  // created/used by RenderSystem

    TextAlign   align  {};  // how to calculate relative offset of text
    vec2        offset {};  // apply arbitrary offset from calculated position

    inline bool isDirty(void)
    {
#if 0
        return
            str != cacheProps.str ||
            font != cacheProps.font ||
            !v4_equals(&color, &cacheProps.color) ||
            strcmp(str, cacheProps.str);
#else
        return dirty;
#endif
    }
};