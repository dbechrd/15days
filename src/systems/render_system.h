#pragma once
#include "../common/command.h"
#include "../common/draw_list.h"
#include "../common/error.h"

struct SDL_Window;
struct SDL_Renderer;

struct RenderSystem {
    FDOVResult Init(const char *title, int width, int height);
    bool Running(void);
    void ProcessCommands(double now, const CommandQueue &commandQueue);
    void Clear(Color color);
    void Render(const DrawList &drawList);
    void Flip(void);
    void Destroy(void);

//private:
    bool          running  {false};
    SDL_Window   *window   {};
    SDL_Renderer *renderer {};
};