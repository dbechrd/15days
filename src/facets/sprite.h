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
    int  animation   {};
    int  frame       {};

    void UpdateBbox(Depot &depot);

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
};

struct Animation {
    int start {};
    int count {};
};

struct Spritesheet : public Facet {
    vec2 cellSize {};
    int  cells    {};
    std::vector<Animation> animations {};
};
