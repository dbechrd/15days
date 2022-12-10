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

void SpriteSystem::React(double now, Depot &depot)
{
    // TODO: Update sprite states based on game events (e.g. combat messages)
}

void SpriteSystem::Behave(double now, Depot &depot, double dt)
{
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
        assert(position);
        if (!position) {
            printf("WARN: Can't draw a sprite with no position");
            continue;
        }

        DrawCommand drawSprite{};
        drawSprite.uid = sprite.uid;
        drawSprite.color = sprite.color;
        drawSprite.rect.x = position->pos.x;
        drawSprite.rect.y = position->pos.y - position->pos.z;
        drawSprite.rect.w = sprite.size.x;
        drawSprite.rect.h = sprite.size.y;
        drawSprite.sprite = sprite.uid;
        drawQueue.push(drawSprite);

        Combat *combat = (Combat *)depot.GetFacet(sprite.uid, Facet_Combat);
        if (combat) {
            if (combat->attackStartedAt) {
                assert(combat->attackCooldown);
                float attackAlpha = (now - combat->attackStartedAt) / combat->attackCooldown;
                float overlayHeight = (1.0 - attackAlpha) * sprite.size.y;

                DrawCommand attackOverlay{};
                attackOverlay.uid = sprite.uid;
                attackOverlay.color = sprite.attackColor;
                attackOverlay.rect = drawSprite.rect;
                attackOverlay.rect.y += sprite.size.y - overlayHeight;
                attackOverlay.rect.h = ceilf(overlayHeight);
                drawQueue.push(attackOverlay);
            }
            if (combat->defendStartedAt) {
                assert(combat->defendCooldown);
                float defendAlpha = (now - combat->defendStartedAt) / combat->defendCooldown;
                float overlayHeight = (1.0 - defendAlpha) * sprite.size.y;

                DrawCommand defendOverlay{};
                defendOverlay.uid = sprite.uid;
                defendOverlay.color = sprite.defendColor;
                defendOverlay.rect = drawSprite.rect;
                defendOverlay.rect.y += sprite.size.y - overlayHeight;
                defendOverlay.rect.h = ceilf(overlayHeight);
                drawQueue.push(defendOverlay);
            }
        }
    }
}