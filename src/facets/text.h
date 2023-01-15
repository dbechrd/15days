#pragma once
#include "facet.h"

#define C_WHITE   "`w"
#define C_RED     "`r"
#define C_GREEN   "`g"
#define C_BLUE    "`b"
#define C_CYAN    "`c"
#define C_MAGENTA "`m"
#define C_YELLOW  "`y"
#define C_BLACK   "`k"

enum TextAlign {
    TextAlign_VTop_HLeft,
    TextAlign_VBottom_HCenter,
};

struct TextCache {
    // values of relevant properties when cached texture was last generated
    UID   font {};
    char *str  {};

    void Destroy(void) {
        free(str);
        *this = {};
    }
};

struct Text : public Facet {
    UID         font   {};  // will invalidate cache
    vec4        color  {};  // will invalidate cache
    const char *str    {};  // will invalidate cache
    TextCache   cache  {};  // created/used by RenderSystem

    TextAlign   align  {};  // how to calculate relative offset of text
    vec2        offset {};  // apply arbitrary offset from calculated position

    inline bool isDirty(void)
    {
        return
            str != cache.str ||
            font != cache.font ||
            strcmp(str, cache.str);
    }
};