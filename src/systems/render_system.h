#pragma once
#include "../common/draw_list.h"
#include "../common/error.h"
#include "../common/message.h"
#include "../facets/depot.h"

struct SDL_Window;
struct SDL_Renderer;

struct RenderSystem {
    FDOVResult Init(const char *title, int width, int height);
    void DestroyDepot(const Depot &depot);
    void Destroy();

    bool Running(void);
    void Clear(vec4 color);

    void React(double now, Depot &depot, const MsgQueue &msgQueue);
    void Behave(double now, Depot &depot, double dt);
    void Flush(DrawQueue &drawQueue);
    void Present(void);

private:
    bool          running     {false};
    SDL_Window   *window      {};
    SDL_Renderer *renderer    {};
};