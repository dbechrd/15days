#include "render_system.h"
#include "../facets/depot.h"

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
    for (Font &font : depot.font) {
        GlyphCache *gc = &font.glyphCache;
        SDL_FreeSurface(gc->atlasSurface);
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
                SDL_RenderSetVSync(renderer, vsync);
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
                GlyphCache *gc = &font->glyphCache;

                for (const char *c = text.str; *c; c++) {
                    // Discard whitespace
                    switch (*c) {
                        case '\n': continue;
                        //case ' ': continue;
                    }

                    // Add new glyphs to cache
                    if (!gc->rects.contains((uint32_t)*c)) {
                        SDL_DestroyTexture(gc->atlasTexture);
                        gc->atlasTexture = 0;

                        SDL_Surface *glyph = TTF_RenderGlyph32_Blended(font->ttf_font, *c, { 255, 255, 255, 255 });
                        SDL_SetSurfaceBlendMode(glyph, SDL_BLENDMODE_NONE);

                        if (!gc->atlasSurface) {
                            // Create new surface
                            gc->atlasSurface = SDL_CreateRGBSurfaceWithFormat(
                                0,
                                (int)gc->minSize.w,
                                (int)gc->minSize.h,
                                32,
                                SDL_PIXELFORMAT_BGRA32
                            );
                            SDL_SetSurfaceBlendMode(gc->atlasSurface, SDL_BLENDMODE_NONE);
                        } else if (gc->cursor.x + glyph->w + gc->padding > gc->atlasSurface->w) {
                            // line filled, go to next line
                            gc->cursor.x = gc->padding;
                            gc->cursor.y += gc->lineHeight + gc->padding;
                            gc->lineHeight = 0;
                        }

                        // NOTE: This could also happen in the middle of a line for taller characters
                        if (gc->cursor.y + glyph->h + gc->padding > gc->atlasSurface->h) {
                            // Resize existing surface
                            SDL_Surface *newSurface = SDL_CreateRGBSurfaceWithFormat(
                                0,
                                (int)(gc->atlasSurface->w),
                                (int)(gc->atlasSurface->h * gc->growthFactor),
                                32,
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
                            SDL_FreeSurface(gc->atlasSurface);
                            gc->atlasSurface = newSurface;
                        }

                        rect dstRect{};
                        dstRect.x = gc->cursor.x;
                        dstRect.y = gc->cursor.y;
                        dstRect.w = glyph->w;
                        dstRect.h = glyph->h;

                        gc->rects[*c] = dstRect;
                        // TODO: Padding?
                        gc->cursor.x += glyph->w + gc->padding;
                        gc->lineHeight = MAX(gc->lineHeight, glyph->h);

                        // WE HAVE: 377888772 = SDL_PIXELFORMAT_BGRA8888 = SDL_PIXELFORMAT_ARGB32
                        // WE WANT: 372645892 = SDL_PIXELFORMAT_ARGB8888 = SDL_PIXELFORMAT_BGRA32

                        SDL_Rect fuckSDLsrc = glyph->clip_rect;
                        SDL_Rect fuckSDLdst = { (int)dstRect.x, (int)dstRect.y, (int)dstRect.w, (int)dstRect.h };
                        if (SDL_BlitSurface(glyph, &fuckSDLsrc, gc->atlasSurface, &fuckSDLdst) < 0) {
                        //if (SDL_BlitSurface(glyph, NULL, gc->atlasSurface, &fuckSDLdst) < 0) {
                            SDL_LogError(0, "Blit failed :(");
                        }

                        SDL_FreeSurface(glyph);
                        glyph = 0;
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

    while (!drawQueue.empty()) {
        const DrawCommand &cmd = drawQueue.top();

        SDL_Rect srcRect{ (int)cmd.srcRect.x, (int)cmd.srcRect.y, (int)cmd.srcRect.w, (int)cmd.srcRect.h };
        SDL_FRect dstRect{ cmd.dstRect.x, cmd.dstRect.y, cmd.dstRect.w, cmd.dstRect.h };


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
                SDL_RenderCopyF(renderer, cmd.texture, &srcRect, &dstRect);
            } else {
                SDL_RenderCopyF(renderer, cmd.texture, NULL, &dstRect);
            }
        } else {
            SDL_SetRenderDrawColor(renderer, cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a);
            SDL_RenderFillRectF(renderer, &dstRect);
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
            SDL_FPoint points[] = {
                { dstRect.x            , dstRect.y + dstRect.h },
                { dstRect.x + dstRect.w, dstRect.y + dstRect.h },
                { dstRect.x + dstRect.w, dstRect.y             },
                { dstRect.x            , dstRect.y             },
                { dstRect.x            , dstRect.y + dstRect.h },
            };

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawLinesF(renderer, points, ARRAY_SIZE(points));
        }
#endif

        drawQueue.pop();
    }
}

void RenderSystem::Present(void)
{
    SDL_RenderPresent(renderer);
}

void RenderSystem::InitTexture(Texture &texture, const char *filename)
{
    SDL_Surface *surface = SDL_LoadBMP(filename);
    if (surface) {
        texture.filename = filename;
        texture.size = { (float)surface->w, (float)surface->h };
        texture.sdl_texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    } else {
        printf("Failed to load texture: %s\n  %s\n", filename, SDL_GetError());
    }
}