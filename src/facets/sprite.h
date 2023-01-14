#pragma once
#include "facet.h"

struct Depot;

struct Sprite : public Facet {
    //vec2 ssize  {};
    vec4 color  {};  // 0.0f - 255.0f
    //float scale {};

    void SetSpritesheet(Depot &depot, UID uidSpritesheet);
    void SetAnimIndex(Depot &depot, int animIdx);
    void SetAnimFrame(Depot &depot, int animFrame);

private:
    // These are private because they affect the bbox which gets updated on-demand
    UID  spritesheet {};
    SDL_Texture *cached_sdl_texture {};  // idk if this is safe, but it saves two GetFacet() calls per card, per frame

    int  animation   {};
    int  frame       {};
    rect srcRect     {};

    void UpdateRect(Depot &depot);

public:
    inline UID GetSpritesheet(void) {
        return spritesheet;
    }
    inline int GetAnimIndex(void) {
        return animation;
    }
    inline int GetAnimFrame(void) {
        return frame;
    }
    inline rect GetSrcRect(void) {
        return srcRect;
    }
    SDL_Texture *GetSDLTexture(void) {
        return cached_sdl_texture;
    }
};

struct Animation {
    int start {};
    int count {};
};

struct Spritesheet : public Facet {
    UID  texture  {};
    int  cells    {};
    vec2 cellSize {};
    std::vector<Animation> animations {};
};
