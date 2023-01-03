#pragma once
#include "facet.h"

typedef std::unordered_map<uint32_t, rect> RectMap;

struct GlyphCache {
    RectMap rects        {};                    // if contains, then glyph already in atlas
    vec2    minSize      {512, 32};             // min atlas size
    float   padding      {2.0f};                // padding between glyphs
    vec2    cursor       { padding, padding };  // where to write next glyph in atlas
    float   lineHeight   {};                    // height of currently active line
    float   growthFactor {2.0f};                // how much to grow atlas on resize

    SDL_Surface *atlasSurface {};  // glyph atlas in main RAM
    SDL_Texture *atlasTexture {};  // GPU texture
};

struct Font : public Facet {
    const char *filename      {};
    int         ptsize        {};
    int         outline       {};
    int         outlineOffset {};  // useful for off-center drop shadow effect
    TTF_Font   *ttf_font      {};
    GlyphCache  glyphCache    {};
};