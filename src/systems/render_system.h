#pragma once
#include "../common/basic.h"
#include "../common/draw_list.h"
#include "../common/error.h"
#include "../facets/texture.h"

struct SDL_Window;
struct SDL_Renderer;

struct RenderSystem {
    FDOVResult Init(const char *title, int width, int height);
    void DestroyDepot(Depot &depot);
    void Destroy();

    UID LoadTexture_BMP(Depot &depot, const char *filename);

    bool Running(void);
    void Clear(vec4 color);
    void Shake(Depot &depot, float amount, float freq, double duration);

    void React(Depot &depot);
    void UpdateCachedTextures(Depot &depot);
    void Flush(Depot &depot, DrawQueue &drawQueue);
    void Present(void);

    void InitTexture(Texture &texture, const char *filename);
private:
    bool          vsync       {FDOV_VSYNC};
    bool          running     {false};
    int           dbgFontIdx  {0};
    SDL_Window   *window      {};
    SDL_Renderer *renderer    {};
    double        shakeStartedAt {};
    double        shakeDuration  {};
    float         shakeAmount    {};
    float         shakeFreq      {};
};