#include "sprite_system.h"
#include "../facets/depot.h"

void SpriteSystem::InitSprite(Sprite &sprite, vec2 size, vec4 color)
{
    sprite.size = size;
    sprite.color = color;
    sprite.attackColor = C255(COLOR_RED); // { 150, 70, 70, 255 };
    sprite.attackColor.a = 128;
    sprite.defendColor = C255(COLOR_DODGER); //{ 70, 70, 150, 255 };
    sprite.defendColor.a = 128;
}

void SpriteSystem::Update(double now, Depot &depot)
{
    // TODO: Update sprite states based on game events (e.g. combat messages)
    size_t size = depot.msgQueue.size();
    for (int i = 0; i < size; i++) {
        Message msg = depot.msgQueue[i];
        Sprite *sprite = (Sprite *)depot.GetFacet(msg.uid, Facet_Sprite);
        if (!sprite) {
            continue;
        }

        switch (msg.type) {
            case MsgType_Sprite_UpdateAnimation:
            {
                sprite->animation = msg.data.sprite_updateanimation.animation;
                break;
            }
            default: break;
        }
    }

    // Update animated sprites
    if (now - lastAnimAt >= fixedAnimDt) {
        for (Sprite &sprite : depot.sprite) {
            Spritesheet *sheet = (Spritesheet *)depot.GetFacet(sprite.spritesheet, Facet_Spritesheet);
            if (sheet) {
                Animation &anim = sheet->animations[sprite.animation];
                sprite.frame = (sprite.frame + 1) % anim.count;
            }
        }
        lastAnimAt = now;
    }
}

void SpriteSystem::Display(double now, Depot &depot, DrawQueue &drawQueue)
{
    for (Sprite &sprite : depot.sprite) {
        Position *position = (Position *)depot.GetFacet(sprite.uid, Facet_Position);
        DLB_ASSERT(position);
        if (!position) {
            printf("WARN: Can't draw a sprite with no position");
            continue;
        }

        float depth = position->pos.y - position->pos.z + sprite.size.y;
        for (Cursor &cursor : depot.cursor) {
            if (cursor.uidDragSubject == sprite.uid) {
                depth = SCREEN_H * 2.0f;
            }
        }

        rect spriteRect{};
        spriteRect.x = position->pos.x;
        spriteRect.y = position->pos.y - position->pos.z;
        spriteRect.w = sprite.size.x;
        spriteRect.h = sprite.size.y;

        DrawCommand drawSprite{};
        drawSprite.uid = sprite.uid;
        drawSprite.color = sprite.color;
        drawSprite.rect = spriteRect;
        drawSprite.sprite = sprite.uid;
        drawSprite.depth = depth;
        drawQueue.push(drawSprite);
    }
}