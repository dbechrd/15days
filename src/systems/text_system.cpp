#include "text_system.h"
#include "../facets/depot.h"

void TextSystem::React(double now, Depot &depot)
{
    size_t size = depot.msgQueue.size();
    for (int i = 0; i < size; i++) {
        Message &msg = depot.msgQueue[i];
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

void TextSystem::Behave(double now, Depot &depot, double dt)
{

}

void TextSystem::Display(double now, Depot &depot, DrawQueue &drawQueue)
{
    for (Text &text : depot.text) {
        if (!text.str) {
            continue;
        }

        Position *position = (Position *)depot.GetFacet(text.uid, Facet_Position);
        if (!position) {
            printf("WARN: Can't draw text with no position");
            continue;
        }

        float x = position->pos.x + text.offset.x;
        float y = position->pos.y - position->pos.z + text.offset.y;
        float w = text.cache.textureSize.w;
        float h = text.cache.textureSize.h;
        float halfW = w * 0.5f;
        //float halfH = h * 0.5f;

        switch (text.align) {
            case TextAlign_VTop_HLeft: {
                // no-op
                break;
            }
            case TextAlign_VBottom_HCenter: {
                x -= halfW;
                y -= h;
                break;
            }
        }

        DrawCommand drawText{};
        drawText.uid = text.uid;
        drawText.color = text.color;
        // TODO: TextAlign (always centered along x and y for now)
        drawText.rect.x = x;
        drawText.rect.y = y;
        drawText.rect.w = w;
        drawText.rect.h = h;
        drawText.tex = text.cache.texture;
        drawQueue.push(drawText);
    }
}