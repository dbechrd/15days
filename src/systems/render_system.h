#pragma once
#include "../common/draw_list.h"
#include "../common/error.h"
#include "../facets/depot.h"

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

private:
    bool          running     {false};
    SDL_Window   *window      {};
    SDL_Renderer *renderer    {};
};