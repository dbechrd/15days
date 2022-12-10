#include "combat_system.h"
#include "../facets/depot.h"

void CombatSystem::Display(double now, Depot &depot, DrawQueue &drawQueue)
{
    for (Combat &combat : depot.combat) {
        Position *position = (Position *)depot.GetFacet(combat.uid, Facet_Position);
        DLB_ASSERT(position);
        if (!position) {
            printf("WARN: Can't draw combat overlay with no position");
            continue;
        }

        Sprite *sprite = (Sprite *)depot.GetFacet(combat.uid, Facet_Sprite);
        DLB_ASSERT(sprite);
        if (!sprite) {
            printf("WARN: Can't draw combat overlay with no sprite");
            continue;
        }

        float depth = position->pos.y - position->pos.z + sprite->size.y;
        for (Cursor &cursor : depot.cursor) {
            if (cursor.uidDragSubject == sprite->uid) {
                depth = SCREEN_H * 2.0f;
            }
        }

        rect spriteRect{};
        spriteRect.x = position->pos.x;
        spriteRect.y = position->pos.y - position->pos.z;
        spriteRect.w = sprite->size.x;
        spriteRect.h = sprite->size.y;

        if (combat.attackStartedAt) {
            DLB_ASSERT(combat.attackCooldown);
            float attackAlpha = (now - combat.attackStartedAt) / combat.attackCooldown;
            float overlayHeight = (1.0 - attackAlpha) * sprite->size.y;

            DrawCommand attackOverlay{};
            attackOverlay.uid = sprite->uid;
            attackOverlay.color = sprite->attackColor;
            attackOverlay.rect = spriteRect;
            attackOverlay.rect.y += sprite->size.y - overlayHeight;
            attackOverlay.rect.h = ceilf(overlayHeight);
            attackOverlay.depth = depth + 0.001f;
            drawQueue.push(attackOverlay);
        }
        if (combat.defendStartedAt) {
            assert(combat.defendCooldown);
            float defendAlpha = (now - combat.defendStartedAt) / combat.defendCooldown;
            float overlayHeight = (1.0 - defendAlpha) * sprite->size.y;

            DrawCommand defendOverlay{};
            defendOverlay.uid = sprite->uid;
            defendOverlay.color = sprite->defendColor;
            defendOverlay.rect = spriteRect;
            defendOverlay.rect.y += sprite->size.y - overlayHeight;
            defendOverlay.rect.h = ceilf(overlayHeight);
            defendOverlay.depth = depth + 0.001f;
            drawQueue.push(defendOverlay);
        }
    }
}