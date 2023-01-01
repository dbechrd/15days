#include "combat_system.h"
#include "../facets/depot.h"

void CombatSystem::Display(Depot &depot, DrawQueue &drawQueue)
{
    for (Combat &combat : depot.combat) {
        Position *position = (Position *)depot.GetFacet(combat.uid, Facet_Position);
        DLB_ASSERT(position);
        if (!position) {
            printf("WARN: Can't draw combat overlay with no position");
            continue;
        }

        float depth = position->pos.y - position->pos.z + position->size.y;
        for (Cursor &cursor : depot.cursor) {
            if (cursor.uidDragSubject == combat.uid) {
                depth = SCREEN_H * 2.0f;
            }
        }

        rect bbox{};
        bbox.x = position->pos.x;
        bbox.y = position->pos.y - position->pos.z;
        bbox.w = position->size.x;
        bbox.h = position->size.y;

        if (combat.attackStartedAt) {
            DLB_ASSERT(combat.attackCooldown);
            float attackAlpha = (depot.Now() - combat.attackStartedAt) / combat.attackCooldown;
            float overlayHeight = (1.0 - attackAlpha) * position->size.y;

            DrawCommand attackOverlay{};
            attackOverlay.uid = combat.uid;
            attackOverlay.color = C255(COLOR_RED); // { 150, 70, 70, 255 };
            attackOverlay.color.a = 128;
            attackOverlay.dstRect = bbox;
            attackOverlay.dstRect.y += position->size.y - overlayHeight;
            attackOverlay.dstRect.h = ceilf(overlayHeight);
            attackOverlay.depth = depth + 0.001f;
            drawQueue.push_back(attackOverlay);
        }
        if (combat.defendStartedAt) {
            assert(combat.defendCooldown);
            float defendAlpha = (depot.Now() - combat.defendStartedAt) / combat.defendCooldown;
            float overlayHeight = (1.0 - defendAlpha) * position->size.y;

            DrawCommand defendOverlay{};
            defendOverlay.uid = combat.uid;
            defendOverlay.color = C255(COLOR_DODGER); //{ 70, 70, 150, 255 };
            defendOverlay.color.a = 128;
            defendOverlay.dstRect = bbox;
            defendOverlay.dstRect.y += position->size.y - overlayHeight;
            defendOverlay.dstRect.h = ceilf(overlayHeight);
            defendOverlay.depth = depth + 0.001f;
            drawQueue.push_back(defendOverlay);
        }
    }

    std::sort(drawQueue.begin(), drawQueue.end());
}