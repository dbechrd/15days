#include "text_system.h"
#include "../facets/depot.h"

void TextSystem::React(double now, Depot &depot)
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

void TextSystem::Display(double now, Depot &depot, DrawQueue &drawQueue)
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
        while (*c) {
            switch (*c) {
                case '\n': {
                    cursor.x = x;
                    cursor.y += lineHeight;
                    c++;
                    continue;
                }
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

            // Draw drop shadow
            drawGlyph.dstRect = drawRect;
            drawGlyph.dstRect.x += 1 + font->ptsize / 32;
            drawGlyph.dstRect.y += 1 + font->ptsize / 32;
            const float darkenFactor = 0; //0.33f;
            drawGlyph.color = {
                text.color.r * darkenFactor,
                text.color.g * darkenFactor,
                text.color.b * darkenFactor,
                text.color.a
            };
            drawGlyph.depth = 0;
            drawQueue.push(drawGlyph);

            // Drop glyph
            drawGlyph.dstRect = drawRect;
            drawGlyph.color = text.color;
            drawGlyph.depth = 1;
            drawQueue.push(drawGlyph);

            cursor.x += glyphRect.w;
            lineHeight = MAX(lineHeight, glyphRect.h);
            c++;
        }
    }

    int atlasOffsetY = 10;

    for (Font &font : depot.font) {
        if (!font.glyphCache.atlasSurface)
            continue;

        rect rect{};
        rect.x = SCREEN_W - font.glyphCache.atlasSurface->w - 10;
        rect.y = atlasOffsetY;
        rect.w = font.glyphCache.atlasSurface->w;
        rect.h = font.glyphCache.atlasSurface->h;

        DrawCommand drawGlyphAtlasBg{};
        drawGlyphAtlasBg.uid = font.uid;
        drawGlyphAtlasBg.dstRect = rect;
        drawGlyphAtlasBg.color = C255(COLOR_BLACK);
        drawGlyphAtlasBg.depth = 0;
        drawQueue.push(drawGlyphAtlasBg);

        DrawCommand drawGlyphAtlas{};
        drawGlyphAtlas.uid = font.uid;
        drawGlyphAtlas.dstRect = rect;
        drawGlyphAtlas.texture = font.glyphCache.atlasTexture;
        drawGlyphAtlas.depth = 1;
        drawQueue.push(drawGlyphAtlas);

        atlasOffsetY += rect.h + 10;
    }
}