#include "render_system.h"
#include "../common/draw_list.h"
#include "SDL/SDL.h"
#include <cstdio>

FDOVResult RenderSystem::Init(const char *title, int width, int height)
{
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "true");

    int sdl_init_err = SDL_Init(SDL_INIT_TIMER | SDL_INIT_EVENTS | SDL_INIT_VIDEO);
    if (sdl_init_err < 0) {
        printf("Failed to initialize SDL video: %s\n", SDL_GetError());
        return FDOV_INIT_FAILED;
    }

    window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_OPENGL
    );
    if (!window) {
        printf("Failed to create window: %s\n", SDL_GetError());
        return FDOV_INIT_FAILED;
    }

#if 0
    int videoDrivers = SDL_GetNumVideoDrivers();
    for (int i = 0; i < videoDrivers; i++) {
        printf("video_driver[%d] = %s\n", i, SDL_GetVideoDriver(i));
    }

    int drivers = SDL_GetNumRenderDrivers();
    for (int i = 0; i < drivers; i++) {
        SDL_RendererInfo info{};
        if (SDL_GetRenderDriverInfo(i, &info) < 0) {
            printf("Failed to query render_driver[%d] info: %s\n", i, SDL_GetError());
            continue;
        }

        printf("render_driver[%d]:\n"
            "  name                : %s\n"
            "  num_texture_formats : %u\n"
            "  max_texture_width   : %d\n"
            "  max_texture_height  : %d\n",
            i,
            info.name,
            info.num_texture_formats,
            info.max_texture_width,
            info.max_texture_height
        );
    }
#endif

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED); // | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        return FDOV_INIT_FAILED;
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    printf("Video driver: %s\n", SDL_GetCurrentVideoDriver());

    running = true;
    return FDOV_SUCCESS;
}

bool RenderSystem::Running(void)
{
    return running;
}

void RenderSystem::ProcessCommands(double now, const CommandQueue &commandQueue)
{
    for (const CommandType &command : commandQueue) {
        switch (command) {
            case Command_QuitRequested:
            {
                running = false;
                break;
            }
            default: break;
        }
    }
}

void RenderSystem::Clear(Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(renderer);
}

void RenderSystem::Render(const DrawList &drawList)
{
    // TODO: Sort drawQueue by depth (or better.. insert in order instead, eh?)

    for (const DrawCommand &cmd : drawList) {
        SDL_SetRenderDrawColor(renderer, cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a);
        SDL_Rect rect{};
        rect.x = cmd.rect.x;
        rect.y = cmd.rect.y;
        rect.w = cmd.rect.w;
        rect.h = cmd.rect.h;
        SDL_RenderFillRect(renderer, &rect);
    }
}

void RenderSystem::Flip(void)
{
    SDL_RenderPresent(renderer);
}

void RenderSystem::Destroy(void)
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}