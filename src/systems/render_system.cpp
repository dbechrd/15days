#include "render_system.h"
#include "../common/draw_list.h"
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"
#include <cstdio>

FDOVResult RenderSystem::Init(const char *title, int width, int height)
{
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "true");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");

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

void RenderSystem::Clear(vec4 color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(renderer);
}

void RenderSystem::React(double now, Depot &depot, const MsgQueue &msgQueue)
{
    for (const Message &msg : msgQueue) {
        switch (msg.type) {
            case MsgType_Input_Quit:
            {
                running = false;
                break;
            }
            default: break;
        }
    }
}

void RenderSystem::Behave(double now, Depot &depot, double dt)
{
    // Update textures for dirty text caches
    for (Text &text : depot.text) {
        if (text.isDirty()) {
            // TTF_STYLE_NORMAL        0x00
            // TTF_STYLE_BOLD          0x01
            // TTF_STYLE_ITALIC        0x02
            // TTF_STYLE_UNDERLINE     0x04
            // TTF_STYLE_STRIKETHROUGH 0x08
            //TTF_SetFontStyle(font, TTF_STYLE_NORMAL);

            SDL_Surface *surface = TTF_RenderText_Blended(text.font, text.text, { 255, 255, 255, 255 });
            SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_DestroyTexture(text.cache.tex);

            text.cache.font = text.font;
            text.cache.text = text.text;
            text.cache.color = text.color;
            text.cache.tex = texture;
            text.cache.texSize = { (float)surface->w, (float)surface->h };
            SDL_FreeSurface(surface);
        }
    }
}

void RenderSystem::Flush(DrawQueue &drawQueue)
{
    while (!drawQueue.empty()) {
        const DrawCommand &cmd = drawQueue.top();

        SDL_Rect rect{};
        rect.x = cmd.rect.x;
        rect.y = cmd.rect.y;
        rect.w = cmd.rect.w;
        rect.h = cmd.rect.h;

        if (cmd.tex) {
            SDL_SetTextureColorMod(cmd.tex, cmd.color.r, cmd.color.g, cmd.color.b);
            SDL_SetTextureAlphaMod(cmd.tex, cmd.color.a);
            SDL_RenderCopy(renderer, cmd.tex, NULL, &rect);
        } else {
            SDL_SetRenderDrawColor(renderer, cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a);
            SDL_RenderFillRect(renderer, &rect);
        }

        drawQueue.pop();
    }
}

void RenderSystem::Present(void)
{
    SDL_RenderPresent(renderer);
}

void RenderSystem::DestroyDepot(const Depot &depot)
{
    for (const Text &text : depot.text) {
        SDL_DestroyTexture(text.cache.tex);
    }
}

void RenderSystem::Destroy(void)
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}