#include "render_system.h"
#include "../common/draw_list.h"
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"
#include <cstdio>

FDOVResult RenderSystem::Init(const char *title, int width, int height)
{
#if FDOV_VSYNC
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "true");
#endif
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");

    int sdl_init_err = SDL_Init(SDL_INIT_TIMER | SDL_INIT_EVENTS | SDL_INIT_VIDEO);
    if (sdl_init_err < 0) {
        printf("Failed to initialize SDL video: %s\n", SDL_GetError());
        return FDOV_INIT_FAILED;
    }

    int flags = SDL_WINDOW_OPENGL;
#if FDOV_FULLSCREEN
    flags |= SDL_WINDOW_FULLSCREEN;
#endif
    window = SDL_CreateWindow(title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        flags
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

    renderer = SDL_CreateRenderer(window, -1, 0); //SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        return FDOV_INIT_FAILED;
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    printf("Video driver: %s\n", SDL_GetCurrentVideoDriver());

    running = true;
    return FDOV_SUCCESS;
}

void RenderSystem::DestroyDepot(Depot &depot)
{
    for (Text &text : depot.text) {
        text.cache.Destroy();
    }
}

void RenderSystem::Destroy(void)
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
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

void RenderSystem::React(double now, Depot &depot)
{
    for (const Message &msg : depot.msgQueue) {
        switch (msg.type) {
            case MsgType_Render_Quit:
            {
                running = false;
                break;
            }
            case MsgType_Render_ToggleVsync:
            {
                static int vsync = FDOV_VSYNC;
                vsync = !vsync;
                SDL_RenderSetVSync(renderer, vsync);
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

            text.cache.Destroy();
            if (!text.str) {
                continue;
            }

            text.cache.font = text.font;
            text.cache.color = text.color;

            size_t strLen = strlen(text.str);
            text.cache.str = (char *)malloc(strlen(text.str));
            memcpy(text.cache.str, text.str, strLen);

            SDL_Surface *surface = TTF_RenderText_Blended_Wrapped(text.font, text.str, { 255, 255, 255, 255 }, 300);
            text.cache.texture = SDL_CreateTextureFromSurface(renderer, surface);
            text.cache.textureSize = { (float)surface->w, (float)surface->h };
            SDL_FreeSurface(surface);
        }
    }
}

void RenderSystem::Flush(Depot &depot, DrawQueue &drawQueue)
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
#if 0
        vec2 verts[] = {
            { (float)rect.x         , (float)rect.y + rect.h },
            { (float)rect.x + rect.w, (float)rect.y + rect.h },
            { (float)rect.x + rect.w, (float)rect.y },
            { (float)rect.x         , (float)rect.y },
        };

        vec4 colors_f32[4] = {
            C255(COLOR_RED),
            C255(COLOR_GREEN),
            C255(COLOR_BLUE),
            C255(COLOR_YELLOW)
        };

        SDL_Color colors_u8[4]{};
        for (int i = 0; i < 4; i++) {
            colors_u8[i] = {
                (Uint8)colors_f32[i].r,
                (Uint8)colors_f32[i].g,
                (Uint8)colors_f32[i].b,
                (Uint8)colors_f32[i].a
            };
        }

        vec2 uvs[] = {
            { 0, 0 },
            { 1, 0 },
            { 1, 1 },
            { 0, 1 }
        };

        int indices[] = { 0, 1, 2, 0, 2, 3 };

        int err = SDL_RenderGeometryRaw(renderer,
            0, //cmd.tex,
            (float *)verts, sizeof(*verts),
            colors_u8, sizeof(*colors_u8),
            (float *)uvs, sizeof(*uvs),
            ARRAY_SIZE(verts),
            (void *)indices, ARRAY_SIZE(indices), sizeof(*indices));
        if (err) {
            printf("ERROR: Failed to render raw geometry: [%d] %s\n", err, SDL_GetError());
        }
#endif

#if FDOV_DEBUG_BBOX
        SDL_Point points[] = {
            { rect.x         , rect.y + rect.h },
            { rect.x + rect.w, rect.y + rect.h },
            { rect.x + rect.w, rect.y },
            { rect.x         , rect.y },
            { rect.x         , rect.y + rect.h },
        };

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawLines(renderer, points, ARRAY_SIZE(points));
#elif FDOV_DRAG_BBOX
        bool uidBeingDragged = false;
        for (Cursor &cursor : depot.cursor) {
            if (cursor.uidDragSubject == cmd.uid) {
                uidBeingDragged = true;
                break;
            }
        }

        if (uidBeingDragged) {
            SDL_Point points[] = {
                { rect.x         , rect.y + rect.h },
                { rect.x + rect.w, rect.y + rect.h },
                { rect.x + rect.w, rect.y },
                { rect.x         , rect.y },
                { rect.x         , rect.y + rect.h },
            };

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawLines(renderer, points, ARRAY_SIZE(points));
        }
#endif

        drawQueue.pop();
    }
}

void RenderSystem::Present(void)
{
    SDL_RenderPresent(renderer);
}