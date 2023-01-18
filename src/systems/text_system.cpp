#include "text_system.h"
#include "../facets/depot.h"

Font *TextSystem::FindOrLoadFont(Depot &depot, const char *fontKey)
{
    // Check if already loaded
    Font *existingFont = (Font *)depot.GetFacetByName(fontKey, Facet_Font);
    if (existingFont) {
        return existingFont;
    }

    const char *ttf_path = 0;
    int point_size = 0;

    const ResourceDB::Font *dbFont = depot.resources->fonts()->LookupByKey(fontKey);
    if (dbFont) {
        ttf_path = dbFont->ttf_path()->c_str();
        point_size = dbFont->point_size();
    } else {
        SDL_LogError(0, "Invalid font key: %s", fontKey);
    }

    TTF_Font *ttfFont = TTF_OpenFont(ttf_path, point_size);
    if (!ttfFont) {
        SDL_LogError(0, "Failed to load font %s\n  %s\n", ttf_path, TTF_GetError());
        return 0;
    }

    UID uidFont = depot.Alloc(fontKey);
    Font *font = (Font *)depot.AddFacet(uidFont, Facet_Font);
    font->fontKey = fontKey;
#if 0
    font->outline = 0;
    font->outlineOffset = { 2, 1 };
    if (ptsize >= 32) {
        font->outlineOffset.x += 1;
        font->outlineOffset.y += 1;
    }
#else
    font->outline = 1;
    font->outlineOffset = { 0 };
#endif
    font->ttf_font = ttfFont;
    return font;
}

void TextSystem::React(Depot &depot)
{
    size_t size = depot.msgQueue.size();
    for (int i = 0; i < size; i++) {
        Message msg = depot.msgQueue[i];
        Text *text = (Text *)depot.GetFacet(msg.uid, Facet_Text);
        if (!text) {
            continue;
        }

        switch (msg.type) {
            case MsgType_Text_UpdateText:
            {
                text->str = msg.data.text_updatetext.str;
                text->offset = msg.data.text_updatetext.offset;
                text->color = msg.data.text_updatetext.color;
                break;
            }
            default: break;
        }
    }
}

void TextSystem::Display(Depot &depot, DrawQueue &drawQueue)
{
    static bool debugKern = false;

    for (Text &text : depot.text) {
        if (!text.str) {
            continue;
        }

        Position *position = (Position *)depot.GetFacet(text.uid, Facet_Position);
        if (!position) {
            printf("WARN: Can't draw text with no position\n");
            continue;
        }

        float x = position->pos.x + text.offset.x;
        float y = position->pos.y - position->pos.z + text.offset.y;

        Font *font = FindOrLoadFont(depot, text.fontKey);
        if (!font) {
            printf("WARN: Can't draw text with no font\n");
            continue;
        }

        bool hasKerning = TTF_GetFontKerning(font->ttf_font);

        // TODO: TextAlign (always centered along x and y for now)

        const char *c = text.str;
        vec2 cursor{ x, y };
        float lineHeight = 0;

        vec4 color = C255(COLOR_WHITE);

        char cPrev = 0;
        while (*c) {
            switch (*c) {
                case '\n': {
                    cursor.x = x;
                    cursor.y += lineHeight;
                    c++;
                    continue;
                }
            }

            char cNext = *(c+1);
            // `` = literal backtick, don't do this color stuffs
            if (*c == '`' && cNext != '`') {
                c++;
                DLB_ASSERT(*c);
                switch (*c) {
                    case C_WHITE[1]:   color = { 255, 255, 255, 255 }; c++; break;
                    case C_RED[1]:     color = { 165,   0,   0, 255 }; c++; break;
                    case C_GREEN[1]:   color = {  49, 165,   0, 255 }; c++; break;
                    case C_BLUE[1]:    color = {  43,  75, 255, 255 }; c++; break;
                    case C_CYAN[1]:    color = {   0, 196, 196, 255 }; c++; break;
                    case C_MAGENTA[1]: color = { 198,  79, 198, 255 }; c++; break;
                    case C_YELLOW[1]:  color = { 234, 199,   0, 255 }; c++; break;
                    case C_BLACK[1]:   color = {   0,   0,   0, 255 }; c++; break;
                    default: printf("WARN: Dats a weird color value, mang\n"); break;
                }
                continue;
            }

            if (!font->glyphCache.rects.contains((uint32_t)*c)) {
                c++;
                continue;
            }

            if (hasKerning) {
                int kern = TTF_GetFontKerningSizeGlyphs32(font->ttf_font, cPrev, *c);
                if (debugKern) {
                    printf("Kern for '%c','%c' = %d\n", cPrev, *c, kern);
                }
                cursor.x += (float)kern;
            }

            rect glyphRect = font->glyphCache.rects[*c];
            DLB_ASSERT(glyphRect.w);
            DLB_ASSERT(glyphRect.h);
            DLB_ASSERT(font->glyphCache.atlasTexture);

            rect drawRect = glyphRect;
            drawRect.x = cursor.x;
            drawRect.y = cursor.y;

            DrawCommand drawGlyph{};
            drawGlyph.uid = text.uid;
            drawGlyph.srcRect = glyphRect;
            drawGlyph.texture = font->glyphCache.atlasTexture;
            drawGlyph.dstRect = drawRect;
            drawGlyph.color = color;
            drawGlyph.depth = 1;
            drawQueue.push_back(drawGlyph);

            cursor.x += glyphRect.w - font->outline;
            lineHeight = MAX(lineHeight, glyphRect.h);
            cPrev = *c;
            c++;
        }
    }

    debugKern = false;
}