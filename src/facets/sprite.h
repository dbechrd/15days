#pragma once
#include "facet.h"

struct Depot;

struct Sprite : public Facet {
    //vec2 ssize  {};
    //float scale {};
    vec4         color       {};  // 0.0f - 255.0f
    UID          spritesheet {};
    const char * animation   {};
    int          frame       {};

    void UpdateRect(Depot &depot);

    inline rect GetSrcRect(void) {
        return srcRect;
    }
    SDL_Texture *GetSDLTexture(void) {
        return cached_sdl_texture;
    }

private:
    SDL_Texture *cached_sdl_texture {};  // idk if this is safe, but it saves two GetFacet() calls per card, per frame
    rect         srcRect            {};
};

struct Animation {
    const char * name  {};
    const char * desc  {};
    int          start {};
    int          count {};
};

struct Spritesheet : public Facet {
    UID  texture  {};
    int  cells    {};
    vec2 cellSize {};
    std::vector<Animation> animations {};
    std::unordered_map<std::string, size_t> animations_by_name {};
};
