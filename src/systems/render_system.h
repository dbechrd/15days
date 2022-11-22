#pragma once
#include "../common/message.h"
#include "../common/draw_list.h"
#include "../common/error.h"

struct SDL_Window;
struct SDL_Renderer;

struct Depot;

struct RenderSystem {
    FDOVResult Init(const char *title, int width, int height);
    bool Running(void);
    void ProcessMessages(double now, Depot &depot, const MsgQueue &msgQueue);
    void Clear(vec4 color);
    void Render(const DrawList &drawList);
    void Flip(void);
    void Destroy(void);

//private:
    bool          running  {false};
    SDL_Window   *window   {};
    SDL_Renderer *renderer {};
};