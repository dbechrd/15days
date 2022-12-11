#include "collision_system.h"
#include "../facets/depot.h"

rect entity_bbox(Depot &depot, UID uid)
{
    rect bbox{};

    Position *position = (Position *)depot.GetFacet(uid, Facet_Position);
    if (position) {
        bbox.x = position->pos.x;
        bbox.y = position->pos.y - position->pos.z;
        bbox.w = 1;
        bbox.h = 1;
    }
    Sprite *sprite = (Sprite *)depot.GetFacet(uid, Facet_Sprite);
    if (sprite) {
        bbox.w = sprite->size.w;
        bbox.h = sprite->size.h;
    }
#if 0
    Text *text = (Text *)depot.GetFacet(uid, Facet_Text);
    if (text) {
        // TODO: Have to account for both the text offset (in pos check)
        // as well as alignment offsets for this to work correctly. We
        // should make Sprite->GetBBox() and Text->GetBBox() helpers or
        // something.
        Texture *texture = (Texture *)depot.GetFacet(uid, Facet_Texture);
        if (texture) {
            bbox.w = texture->size.w;
            bbox.h = texture->size.h;
        }
    }
#endif
    CardStack *cardStack = (CardStack *)depot.GetFacet(uid, Facet_CardStack);
    if (cardStack) {
        for (UID &uidCard : cardStack->cards) {
            rect cardBbox = entity_bbox(depot, uidCard);
            bbox.x = MIN(bbox.x, cardBbox.x);
            bbox.y = MIN(bbox.y, cardBbox.y);
            bbox.w = MAX(bbox.x + bbox.w, cardBbox.x + cardBbox.w) - bbox.x;
            bbox.h = MAX(bbox.y + bbox.h, cardBbox.y + cardBbox.h) - bbox.y;
        }
    }

    return bbox;
}

void CollisionSystem::DetectCollisions(Depot &depot, CollisionList &collisionList)
{
    for (int i = 0; i < depot.position.size(); i++) {
        UID uidA = depot.position[i].uid;
        const rect bboxA = entity_bbox(depot, uidA);
        if (!bboxA.w || !bboxA.h) {
            continue;
        }

        for (int j = i + 1; j < depot.position.size(); j++) {
            UID uidB = depot.position[j].uid;

            const rect bboxB = entity_bbox(depot, uidB);
            if (!bboxB.w || !bboxB.h) {
                continue;
            }

            if (rect_intersect(&bboxA, &bboxB)) {
                collisionList.push_back({ uidA, uidB, bboxA, bboxB });
            }
        }
    }
}
