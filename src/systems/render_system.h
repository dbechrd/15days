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

    bool Running(void);
    void Clear(vec4 color);

    void React(double now, Depot &depot);
    void Behave(double now, Depot &depot, double dt);
    void Flush(Depot &depot, DrawQueue &drawQueue);
    void Present(void);

    void InitTexture(Texture &texture, const char *filename);
private:
    bool          running     {false};
    SDL_Window   *window      {};
    SDL_Renderer *renderer    {};
};