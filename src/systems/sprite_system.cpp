#include "sprite_system.h"
#include "../facets/depot.h"
#include <cassert>

void SpriteSystem::InitSprite(Sprite &sprite)
{
    sprite.size = { 125, 200 };
    sprite.color = C255(COLOR_WHEAT); // { 15, 70, 90, 255 };
    sprite.attackColor = C255(COLOR_RED); // { 150, 70, 70, 255 };
    sprite.defendColor = C255(COLOR_DODGER); //{ 70, 70, 150, 255 };
}

void SpriteSystem::React(double now, Depot &depot, MsgQueue &msgQueue)
{
    // TODO: Update sprite states based on game events (e.g. combat messages)
}

void SpriteSystem::Behave(double now, Depot &depot, double dt)
{
#if 0
    for (Sprite &sprite : depot.sprite) {
        // TODO: Animation
    }
#endif
}

void SpriteSystem::Display(double now, Depot &depot, DrawQueue &drawQueue)
{
    for (Sprite &sprite : depot.sprite) {
        Position *position = (Position *)depot.GetFacet(sprite.uid, Facet_Position);
        assert(position);
        if (!position) {
            printf("WARN: Can't draw a sprite with no position");
            continue;
        }

        DrawCommand drawSprite{};
        drawSprite.color = sprite.color;
        drawSprite.rect.x = position->pos.x;
        drawSprite.rect.y = position->pos.y;
        drawSprite.rect.w = sprite.size.x;
        drawSprite.rect.h = sprite.size.y;
        drawQueue.push(drawSprite);

        Combat *combat = (Combat *)depot.GetFacet(sprite.uid, Facet_Combat);
        if (combat) {
            if (combat->attackStartedAt) {
                assert(combat->attackCooldown);
                float attackAlpha = (now - combat->attackStartedAt) / combat->attackCooldown;
                float overlayHeight = (1.0 - attackAlpha) * sprite.size.y;

                DrawCommand attackOverlay{};
                attackOverlay.color = sprite.attackColor;
                attackOverlay.rect = drawSprite.rect;
                attackOverlay.rect.y = position->pos.y + sprite.size.y - overlayHeight;
                attackOverlay.rect.h = ceilf(overlayHeight);
                drawQueue.push(attackOverlay);
            }
            if (combat->defendStartedAt) {
                assert(combat->defendCooldown);
                float defendAlpha = (now - combat->defendStartedAt) / combat->defendCooldown;
                float overlayHeight = (1.0 - defendAlpha) * sprite.size.y;

                DrawCommand defendOverlay{};
                defendOverlay.color = sprite.defendColor;
                defendOverlay.rect = drawSprite.rect;
                defendOverlay.rect.y = position->pos.y + sprite.size.y - overlayHeight;
                defendOverlay.rect.h = ceilf(overlayHeight);
                drawQueue.push(defendOverlay);
            }
        }
    }
}