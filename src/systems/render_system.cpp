#include "render_system.h"
#include "../facets/depot.h"

void RenderSystem::InitTexture(Texture &texture, const char *filename)
{
    SDL_Surface *surface = SDL_LoadBMP(filename);
    if (!surface) {
        static SDL_Surface *pinkSquare{};
        if (!pinkSquare) {
            pinkSquare = SDL_CreateSurface(16, 16, SDL_PIXELFORMAT_RGBA8888);
            SDL_FillSurfaceRect(pinkSquare, 0, 0xFF00FFFF);
        }
        surface = pinkSquare;
        filename = "FILE_FAILED_TO_LOAD_PINK_SQUARE";
        printf("Failed to load texture: %s\n  %s\n", filename, SDL_GetError());
    }

    texture.filename = filename;
    texture.size = { (float)surface->w, (float)surface->h };
    texture.sdl_texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
}

FDOVResult RenderSystem::Init(const char *title, int width, int height)
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

    int sdl_init_err = SDL_Init(SDL_INIT_TIMER | SDL_INIT_EVENTS | SDL_INIT_VIDEO);
    if (sdl_init_err < 0) {
        printf("Failed to initialize SDL video: %s\n", SDL_GetError());
        return FDOV_INIT_FAILED;
    }

    int flags = SDL_WINDOW_OPENGL;
    if (fullscreen) {
        flags |= SDL_WINDOW_FULLSCREEN;
    }

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

    renderer = SDL_CreateRenderer(window, 0, 0); //SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        return FDOV_INIT_FAILED;
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
    return FDOV_SUCCESS;
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

UID RenderSystem::LoadTexture_BMP(Depot &depot, const char *filename)
{
    // Check if already loaded
    Texture *existingTexture = (Texture *)depot.GetFacetByName(filename, Facet_Texture);
    if (existingTexture) {
        return existingTexture->uid;
    }

    // Load a new texture
    UID uidTexture = depot.Alloc(filename);
    Texture *texture = (Texture *)depot.AddFacet(uidTexture, Facet_Texture);
    depot.renderSystem.InitTexture(*texture, filename);
    return uidTexture;
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

void RenderSystem::Shake(Depot &depot, float amount, float freq, double duration)
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
            case MsgType_Render_Screenshake:
            {
                Shake(depot,
                    msg.data.render_screenshake.amount,
                    msg.data.render_screenshake.freq,
                    msg.data.render_screenshake.duration);
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
            text.font = depot.font[dbgFontIdx].uid;
        }
        printf("font: %s\n", depot.font[dbgFontIdx].filename);
    }
}

void RenderSystem::UpdateCachedTextures(Depot &depot)
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

            Texture *texture = (Texture *)depot.AddFacet(text.uid, Facet_Texture, false);
            SDL_DestroyTexture(texture->sdl_texture);
            texture->sdl_texture = 0;
            //text.cacheProps.Destroy();
            if (!text.str) {
                continue;
            }

            //text.cacheProps.font = text.font;
            //text.cacheProps.color = text.color;
            //size_t strLen = strlen(text.str);
            //text.cacheProps.str = (char *)calloc(strlen(text.str) + 1, sizeof(*text.cacheProps.str));
            //memcpy((void *)text.cacheProps.str, text.str, strLen);

            Font *font = (Font *)depot.GetFacet(text.font, Facet_Font);
            if (font && font->ttf_font) {
                //DLB_ASSERT(fabs(font->outlineOffset.x <= font->outline));
                //DLB_ASSERT(fabs(font->outlineOffset.y <= font->outline));

                GlyphCache *gc = &font->glyphCache;

                for (const char *c = text.str; *c; c++) {
                    // Discard whitespace
                    switch (*c) {
                        case '\n': continue;
                        //case ' ': continue;
                    }

                    // Add new glyphs to cache
                    if (!gc->rects.contains((uint32_t)*c)) {
#if 0
                        static bool leftDownPrev = 0;
                        bool leftDown = (SDL_GetGlobalMouseState(0, 0) & SDL_BUTTON_LMASK) > 0;
                        bool leftChanged = leftDown != leftDownPrev;
                        bool leftPressed = leftDown && leftChanged;
                        leftDownPrev = leftDown;
                        if (!leftPressed) {
                            continue;
                        }
#endif
                        SDL_DestroyTexture(gc->atlasTexture);
                        gc->atlasTexture = 0;

                        TTF_SetFontOutline(font->ttf_font, font->outline);
                        SDL_Surface *glyphOutline = TTF_RenderGlyph32_Blended(font->ttf_font, *c, { 0, 0, 0, 255 });
                        SDL_SetSurfaceBlendMode(glyphOutline, SDL_BLENDMODE_BLEND);

                        TTF_SetFontOutline(font->ttf_font, 0);
                        SDL_Surface *glyphBody = TTF_RenderGlyph32_Blended(font->ttf_font, *c, { 255, 255, 255, 255 });
                        SDL_SetSurfaceBlendMode(glyphBody, SDL_BLENDMODE_BLEND);

                        // DEBUG: This shows outline color aliasing.. meh how to get rid of it?
                        //SDL_SetSurfaceBlendMode(glyphBody, SDL_BLENDMODE_MUL);

                        rect glyphRect{};
                        glyphRect.x = gc->cursor.x;
                        glyphRect.y = gc->cursor.y;
                        glyphRect.w = MAX(glyphOutline->w + fabs(font->outlineOffset.x), glyphBody->w);
                        glyphRect.h = MAX(glyphOutline->h + fabs(font->outlineOffset.h), glyphBody->h);

                        float neededWidth = gc->cursor.x + glyphRect.w + gc->padding;
                        float neededHeight = gc->cursor.y + glyphRect.h + gc->padding;

                        if (!gc->atlasSurface) {
                            // Create new surface
                            gc->atlasSurface = SDL_CreateSurface(
                                MAX((int)gc->minSize.w, neededWidth),
                                MAX((int)gc->minSize.h, neededHeight),
                                SDL_PIXELFORMAT_BGRA32
                            );
                            SDL_SetSurfaceBlendMode(gc->atlasSurface, SDL_BLENDMODE_NONE);
                        }

                        if (neededWidth > gc->atlasSurface->w) {
                            // line filled, go to next line
                            gc->cursor.x = gc->padding;
                            gc->cursor.y += gc->lineHeight + gc->padding;
                            gc->lineHeight = 0;
                        }

                        // NOTE: This could also happen in the middle of a line for taller characters
                        if (neededWidth > gc->atlasSurface->w || neededHeight > gc->atlasSurface->h) {
                            // Resize existing surface
                            float newWidth  = MAX(gc->atlasSurface->w, neededWidth);
                            float newHeight = MAX(gc->atlasSurface->h * gc->growthFactor, neededHeight);

                            SDL_Surface *newSurface = SDL_CreateSurface(
                                newWidth, newHeight,
                                SDL_PIXELFORMAT_BGRA32
                            );
                            SDL_SetSurfaceBlendMode(newSurface, SDL_BLENDMODE_NONE);

                            SDL_Rect fuckSDLdst = gc->atlasSurface->clip_rect;
                            SDL_Rect fuckSDLsrc = newSurface->clip_rect;
                            SDL_BlitSurface(
                                gc->atlasSurface,
                                &fuckSDLdst,
                                newSurface,
                                &fuckSDLsrc
                            );
                            SDL_DestroySurface(gc->atlasSurface);
                            gc->atlasSurface = newSurface;
                            //SDL_SetSurfaceBlendMode(gc->atlasSurface, SDL_BLENDMODE_BLEND);
                        }

                        // TODO: Padding?
                        gc->cursor.x += glyphRect.w + gc->padding;
                        gc->lineHeight = MAX(gc->lineHeight, glyphRect.h);

                        // WE HAVE: 377888772 = SDL_PIXELFORMAT_BGRA8888 = SDL_PIXELFORMAT_ARGB32
                        // WE WANT: 372645892 = SDL_PIXELFORMAT_ARGB8888 = SDL_PIXELFORMAT_BGRA32

                        SDL_Rect sdlDstGlyphOutline = { (int)glyphRect.x, (int)glyphRect.y, (int)glyphRect.w, (int)glyphRect.h };
                        sdlDstGlyphOutline.x += font->outlineOffset.x;
                        sdlDstGlyphOutline.y += font->outlineOffset.y;
                        if (SDL_BlitSurface(glyphOutline, 0, gc->atlasSurface, &sdlDstGlyphOutline) < 0) {
                            SDL_LogError(0, "Blit failed: %s", SDL_GetError());
                        }

                        SDL_Rect sdlDstGlyphBody = { (int)glyphRect.x, (int)glyphRect.y, (int)glyphRect.w, (int)glyphRect.h };
                        sdlDstGlyphBody.x += font->outline;
                        sdlDstGlyphBody.y += font->outline;
                        if (SDL_BlitSurface(glyphBody, 0, gc->atlasSurface, &sdlDstGlyphBody) < 0) {
                            SDL_LogError(0, "Blit failed: %s", SDL_GetError());
                        }

                        gc->rects[*c] = glyphRect;

                        SDL_DestroySurface(glyphOutline);
                        SDL_DestroySurface(glyphBody);
                        glyphOutline = 0;
                        glyphBody = 0;
#if 0
                        // DEBUG(cleanup): Print glyphs as they're cached
                        printf("%c ", *c);
#endif
                    }
                }

                if (!gc->atlasTexture) {
                    gc->atlasTexture = SDL_CreateTextureFromSurface(renderer, gc->atlasSurface);
                    SDL_SetTextureBlendMode(gc->atlasTexture, SDL_BLENDMODE_BLEND);
                }
            } else {
                SDL_Log("Unable to update text %u, cannot find font for uid %u\n", text.uid, text.font);
            }
        }
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

            SDL_SetRenderDrawColor(renderer, cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a);
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
    SDL_RenderPresent(renderer);
}