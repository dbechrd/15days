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
    const char * fontKey {};
    char *       str     {};

    void Destroy(void) {
        free(str);
        *this = {};
    }
};

struct Text : public Facet {
    const char * fontKey  {};  // will invalidate cache
    const char * str      {};  // will invalidate cache
    bool         strOwner {};  // if true, need to free it
    TextCache    cache    {};  // created/used by RenderSystem

    TextAlign    align    {};  // how to calculate relative offset of text
    vec2         offset   {};  // apply arbitrary offset from calculated position

    inline bool isDirty(void)
    {
        return
            str != cache.str ||
            fontKey != cache.fontKey ||
            strcmp(str, cache.str);
    }
};