#include "text_system.h"
#include "../facets/depot.h"

UID TextSystem::LoadFont(Depot &depot, const char *filename, int ptsize)
{
    size_t keyLen = strlen(filename) + 10;
    char *key = (char *)depot.frameArena.Alloc(keyLen);
    snprintf(key, keyLen, "%s?ptsize=%d", filename, ptsize);

    // Check if already loaded
    Font *existingFont = (Font *)depot.GetFacetByName(key, Facet_Font);
    if (existingFont) {
        return existingFont->uid;
    }

    // Load a new font
    UID uidFont = depot.Alloc(key);
    Font *font = (Font *)depot.AddFacet(uidFont, Facet_Font);
    font->filename = filename;
    font->ptsize = ptsize;
    font->outline = 1;
    font->outlineOffset = { 1, 0 };
    font->ttf_font = TTF_OpenFont(filename, ptsize);
    if (!font->ttf_font) {
        SDL_LogError(0, "Failed to load font %s\n", filename);
    }

    return uidFont;
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

        Font *font = (Font *)depot.GetFacet(text.font, Facet_Font);
        if (!font) {
            printf("WARN: Can't draw text with no font\n");
            continue;
        }

        // TODO: TextAlign (always centered along x and y for now)

        const char *c = text.str;
        vec2 cursor{ x, y };
        float lineHeight = 0;

        vec4 color = C255(COLOR_WHITE);

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
                if (!*c || !isdigit(*c)) {
                    break;
                }
                DLB_ASSERT(*c);
                DLB_ASSERT(isdigit(*c));
                switch (*c) {
                    case '0': color = {   0,   0,   0, 255 }; break;
                    case '1': color = {  43,  75, 255, 255 }; break;
                    case '2': color = {  49, 165,   0, 255 }; break;
                    case '3': color = {   0, 196, 196, 255 }; break;
                    case '4': color = { 165,   0,   0, 255 }; break;
                    case '5': color = { 198,  79, 198, 255 }; break;
                    case '6': color = { 234, 199,   0, 255 }; break;
                    case '7': color = { 255, 255, 255, 255 }; break;
                    default: printf("WARN: Dats a weird color value, mang\n"); break;
                }
                c++;
                continue;
            }

            if (!font->glyphCache.rects.contains((uint32_t)*c)) {
                c++;
                continue;
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
            c++;
        }
    }
}