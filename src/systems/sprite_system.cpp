#include "sprite_system.h"
#include "../facets/depot.h"
#include <cassert>

void SpriteSystem::InitSprite(Sprite &sprite)
{
    sprite.size = { 200, 300 };
    sprite.color = { 15, 50, 70, 255 };
    sprite.attackColor = { 150, 70, 70, 255 };
}

void SpriteSystem::Update(double now, Depot &depot)
{
#if 0
    for (Sprite &sprite : depot.sprite) {
        // TODO: Animation
    }
#endif
}

void SpriteSystem::Draw(double now, Depot &depot, DrawList &drawList)
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
        drawList.push_back(drawSprite);

        Combat *combat = (Combat *)depot.GetFacet(sprite.uid, Facet_Combat);
        if (combat && combat->attackStartedAt) {
            assert(combat->attackDuration);
            float attackAlpha = (now - combat->attackStartedAt) / combat->attackDuration;
            float attackOverlayHeight = (1.0 - attackAlpha) * sprite.size.y;
            if (attackOverlayHeight) {
                DrawCommand drawAttackOverlay{};
                drawAttackOverlay.color = sprite.attackColor;
                drawAttackOverlay.rect = drawSprite.rect;
                drawAttackOverlay.rect.y = position->pos.y + sprite.size.y - attackOverlayHeight;
                drawAttackOverlay.rect.h = ceilf(attackOverlayHeight);
                drawList.push_back(drawAttackOverlay);
            }
        }
    }
}