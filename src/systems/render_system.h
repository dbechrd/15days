#pragma once
#include "../common/basic.h"
#include "../common/draw_list.h"
#include "../facets/texture.h"

struct Msg_Render_FrameBegin {
    double realDtSmooth {};
};

struct RenderSystem {
    Texture *FindOrCreateTextureBMP(Depot &depot, const char *textureKey);

    Error Init(const char *title, int width, int height);
    void DestroyDepot(Depot &depot);
    void Destroy();

    // Use sparingly, inter-system dependencies make the code less readable
    SDL_Window *Window(void) { return window; }
    SDL_Renderer *Renderer(void) { return renderer; }
    vec2 WindowSize(void);

    bool Running(void);
    void Clear(vec4 color);
    void PushShake(Depot &depot, float amount, float freq, double duration);

    void React(Depot &depot);
    void UpdateCachedTextures(Depot &depot);
    void Flush(Depot &depot, DrawQueue &drawQueue);
    void Present(void);

private:
    bool          fullscreen     { FDOV_FULLSCREEN };
    bool          vsync          { FDOV_VSYNC };
    bool          running        { false };
    int           dbgFontIdx     { 0 };
    SDL_Window   *window         {};
    SDL_Renderer *renderer       {};
    double        shakeStartedAt {};
    double        shakeDuration  {};
    float         shakeAmount    {};
    float         shakeFreq      {};
};