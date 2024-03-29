#include "render_system.h"
#include "../facets/depot.h"

Texture *RenderSystem::FindOrCreateTextureBMP(Depot &depot, const char *textureKey)
{
    // Check if already loaded
    Texture *existingTexture = (Texture *)depot.GetFacetByName(textureKey, Facet_Texture);
    if (existingTexture) {
        return existingTexture;
    }

    const ResourceDB::Texture *dbTexture = depot.resources->textures()->LookupByKey(textureKey);
    const char *path = 0;
    if (dbTexture) {
        path = dbTexture->path()->c_str();
    } else {
        SDL_LogError(0, "Invalid texture key: %s", textureKey);
    }

    SDL_Surface *surface = SDL_LoadBMP(path);
    if (!surface) {
        SDL_LogError(0, "Failed to load surface: %s\n  %s\n", path, SDL_GetError());

        static SDL_Surface *pinkSquare{};
        if (!pinkSquare) {
            pinkSquare = SDL_CreateSurface(16, 16, SDL_PIXELFORMAT_RGBA8888);
            if (!pinkSquare) {
                SDL_LogError(0, "Failed to load fallback surface:\n  %s\n", SDL_GetError());
                DLB_ASSERT(!"Missing texture also failed to load.. uh-oh no fallback!");
                return 0;
            }
            SDL_FillSurfaceRect(pinkSquare, 0, 0xFF00FFFF);
        }
        surface = pinkSquare;
    }

    SDL_Texture *sdlTexture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!sdlTexture) {
        SDL_LogError(0, "Failed to create texture from surface: %s\n  %s\n", path, SDL_GetError());
        return 0;
    }
    SDL_DestroySurface(surface);

    UID uidTexture = depot.Alloc(textureKey);
    Texture *texture = (Texture *)depot.AddFacet(uidTexture, Facet_Texture);
    texture->textureKey = textureKey;
    texture->size = { (float)surface->w, (float)surface->h };
    texture->sdl_texture = sdlTexture;
    return texture;
}

Error RenderSystem::Init(const char *title, vec2 windowSize)
{
#if FDOV_VSYNC
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "true");
#endif
    // "direct3d"
    // "direct3d11"
    // "opengl"
    // "opengles2"
    // "software"
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    SDL_SetHint(SDL_HINT_RENDER_BATCHING, "true");

    int sdl_init_err = SDL_Init(SDL_INIT_TIMER | SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    if (sdl_init_err < 0) {
        printf("Failed to initialize SDL video: %s\n", SDL_GetError());
        return E_INIT_FAILED;
    }

    int flags = SDL_WINDOW_OPENGL;
    if (fullscreen) {
        flags |= SDL_WINDOW_FULLSCREEN;
    }

    window = SDL_CreateWindow(title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        (int)windowSize.x, (int)windowSize.y,
        flags
    );
    if (!window) {
        printf("Failed to create window: %s\n", SDL_GetError());
        return E_INIT_FAILED;
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

    renderer = SDL_CreateRenderer(window, 0, 0); //SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        return E_INIT_FAILED;
    }

    SDL_RendererInfo rendererInfo{};
    if (!SDL_GetRendererInfo(renderer, &rendererInfo)) {
        printf("Renderer: %s (batching = %s)\n",
            rendererInfo.name,
            SDL_GetHint(SDL_HINT_RENDER_BATCHING)
        );
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    printf("Video driver: %s\n", SDL_GetCurrentVideoDriver());

    running = true;
    return E_SUCCESS;
}

void RenderSystem::DestroyDepot(Depot &depot)
{
    for (Font &font : depot.font) {
        GlyphCache *gc = &font.glyphCache;
        SDL_DestroySurface(gc->atlasSurface);
        gc->atlasSurface = 0;
        SDL_DestroyTexture(gc->atlasTexture);
        gc->atlasTexture = 0;
    }
    for (Text &text : depot.text) {
        //text.cacheProps.Destroy();
    }
    for (Texture &texture : depot.texture) {
        SDL_DestroyTexture(texture.sdl_texture);
        texture.sdl_texture = 0;
    }
}

void RenderSystem::Destroy(void)
{
    SDL_DestroyRenderer(renderer);
    renderer = 0;
    SDL_DestroyWindow(window);
    window = 0;
    SDL_Quit();
}

vec2 RenderSystem::WindowSize(void)
{
    int x = 0, y = 0;
    SDL_GetWindowSize(window, &x, &y);
    vec2 windowSize{ (float)x, (float)y };
    return windowSize;
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

void RenderSystem::PushShake(Depot &depot, float amount, float freq, double duration)
{
    shakeAmount = amount;
    shakeFreq = freq;
    shakeDuration = duration;
    shakeStartedAt = depot.Now();
}

void RenderSystem::React(Depot &depot)
{
    int oldDbgFontIdx = dbgFontIdx;
    for (const Message &msg : depot.msgQueue) {
        switch (msg.type) {
            case MsgType_Render_Quit:
            {
                running = false;
                break;
            }
            case MsgType_Render_ToggleVsync:
            {
                vsync = !vsync;
                SDL_SetRenderVSync(renderer, vsync);
                break;
            }
            case MsgType_Render_ToggleFullscreen:
            {
                fullscreen = !fullscreen;
#if 0
                SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
#else
                SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
#endif
                break;
            }
            case MsgType_Render_DbgSetFontNext:
            {
                dbgFontIdx++;
                dbgFontIdx %= depot.font.size();
                break;
            }
            case MsgType_Render_DbgSetFontPrev:
            {
                dbgFontIdx--;
                if (dbgFontIdx < 0) {
                    dbgFontIdx = depot.font.size() - 1;
                }
                break;
            }
            default: break;
        }
    }
    if (dbgFontIdx != oldDbgFontIdx) {
        for (Text &text : depot.text) {
            text.fontKey = depot.font[dbgFontIdx].fontKey;
        }
        printf("font: %s\n", depot.font[dbgFontIdx].fontKey);
    }
}

void RenderSystem::Flush(Depot &depot, DrawQueue &drawQueue)
{
    // TODO: Batch draw calls into temp allocator
    // then use SDL_RenderGeometryRaw to draw them all at once!!

    vec2 shake{};

    {
        double shakeAlpha = (depot.Now() - shakeStartedAt) / shakeDuration;
        if (shakeAlpha < 1.0) {
            //float distFromHalf = fabs(shakeAlpha - 0.5f);
            //float shakeMult = (0.5 - distFromHalf) * 2;

            // TODO: Extract this out into a lookup table of shake types / intepolation types
            const vec2 flashBang{ 0.97f, 1.01f };  // freq 200, (1 - alpha)
            const vec2 noisy{ 0.83f, 1.13f };  // freq 200, (1 - alpha)

            vec2 resonance = noisy;

            shake.x += sinf(depot.Now() * shakeFreq * resonance.x) * shakeAmount * (1.0 - shakeAlpha);
            shake.y += cosf(depot.Now() * shakeFreq * resonance.y) * shakeAmount * (1.0 - shakeAlpha);
        } else {
            shakeAmount = 0;
            shakeDuration = 0;
            shakeStartedAt = 0;
            shakeFreq = 0;
        }
    }

    for (const DrawCommand &cmd : drawQueue) {
        SDL_Rect srcRect{ (int)cmd.srcRect.x, (int)cmd.srcRect.y, (int)cmd.srcRect.w, (int)cmd.srcRect.h };
        SDL_FRect dstRect{ cmd.dstRect.x, cmd.dstRect.y, cmd.dstRect.w, cmd.dstRect.h };

        dstRect.x += shake.x;
        dstRect.y += shake.y;

        if (cmd.texture) {
            if (cmd.color.a) {
                SDL_SetTextureColorMod(cmd.texture, cmd.color.r, cmd.color.g, cmd.color.b);
                SDL_SetTextureAlphaMod(cmd.texture, cmd.color.a);
            } else {
                SDL_SetTextureColorMod(cmd.texture, 255, 255, 255);
                SDL_SetTextureAlphaMod(cmd.texture, 255);
            }
            // TODO: Can this be shortened? Does SDL handle zero-width same as NULL?
            if (cmd.srcRect.w && cmd.srcRect.h) {
                SDL_RenderTexture(renderer, cmd.texture, &srcRect, &dstRect);
            } else {
                SDL_RenderTexture(renderer, cmd.texture, NULL, &dstRect);
            }
        } else {
            SDL_SetRenderDrawColor(renderer, cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a);
            SDL_RenderFillRect(renderer, &dstRect);
        }

        bool outline = cmd.outline;
#if FDOV_DEBUG_BBOX
        outline = true;
#elif FDOV_DRAG_BBOX
        for (Cursor &cursor : depot.cursor) {
            if (cursor.uidDragSubject == cmd.uid) {
                outline = true;
                break;
            }
        }
#endif

        if (outline) {
            SDL_FPoint points[] = {
                { cmd.dstRect.x                , cmd.dstRect.y + cmd.dstRect.h },
                { cmd.dstRect.x + cmd.dstRect.w, cmd.dstRect.y + cmd.dstRect.h },
                { cmd.dstRect.x + cmd.dstRect.w, cmd.dstRect.y },
                { cmd.dstRect.x                , cmd.dstRect.y },
                { cmd.dstRect.x                , cmd.dstRect.y + cmd.dstRect.h },
            };

            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a);
            SDL_RenderLines(renderer, points, ARRAY_SIZE(points));
        }

#if 0
        vec2 verts[] = {
            { (float)dstRect.x            , (float)dstRect.y + dstRect.h },
            { (float)dstRect.x + dstRect.w, (float)dstRect.y + dstRect.h },
            { (float)dstRect.x + dstRect.w, (float)dstRect.y },
            { (float)dstRect.x            , (float)dstRect.y },
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
    }
}

void RenderSystem::Present(void)
{
    //SDL_Rect clipRect{};
    //clipRect.x = 0;
    //clipRect.y = 0;
    //clipRect.w = SCREEN_W / 2;
    //clipRect.h = SCREEN_H;
    //SDL_SetRenderClipRect(renderer, &clipRect);

    SDL_RenderPresent(renderer);
}