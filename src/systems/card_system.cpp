#include "card_system.h"
#include "../facets/depot.h"

void CardSystem::UpdateCards(Depot &depot)
{
    for (Card &card : depot.card) {
        if (card.stackParent) {
            Position *position = (Position *)depot.GetFacet(card.uid, Facet_Position);
            DLB_ASSERT(position);

            Position *parentPos = (Position *)depot.GetFacet(card.stackParent, Facet_Position);
            DLB_ASSERT(parentPos);

            vec3 targetPos = parentPos->pos;
            targetPos.y += 30.0f;

            const float lerpFac = 1.0f - powf(1.0f - 0.5f, depot.RealDt() * 60.0f);
            position->pos.x = LERP(position->pos.x, targetPos.x, lerpFac);
            position->pos.y = LERP(position->pos.y, targetPos.y, lerpFac);
            position->pos.z = LERP(position->pos.z, targetPos.z, lerpFac);
        }

        Sprite *sprite = (Sprite *)depot.GetFacet(card.uid, Facet_Sprite);
        if (!sprite) {
            printf("WARN: Can't update a card with no sprite");
            continue;
        }

        CardProto *cardProto = (CardProto *)depot.GetFacet(card.cardProto, Facet_CardProto);
        if (!cardProto) {
            printf("WARN: Can't update a card with no card prototype");
            continue;
        }

        if (card.noClickUntil > depot.Now()) {
            sprite->SetAnimIndex(depot, 2);  // TODO: Clean up magic number, move deck back to 0
            //sprite->color = C255(COLOR_GRAY_5);
        } else if (card.noClickUntil) {
            sprite->SetAnimIndex(depot, cardProto->animation);
            //sprite->color = {};
            card.noClickUntil = 0;

            Message msgTryStack{};
            msgTryStack.type = MsgType_Card_Notify_DragEnd;
            msgTryStack.uid = card.uid;
            depot.msgQueue.push_back(msgTryStack);
        }
    }
}

void CardSystem::UpdateStacks(Depot &depot, const CollisionList &collisionList)
{
    DLB_ASSERT(depot.cursor.size() == 1);

    size_t size = depot.msgQueue.size();
    for (int i = 0; i < size; i++) {
        Message msg = depot.msgQueue[i];

        switch (msg.type) {
            case MsgType_Card_Notify_DragBegin: {
                UID draggedCard = msg.uid;

                Card *card = (Card *)depot.GetFacet(draggedCard, Facet_Card);
                if (!card) {
                    continue;
                }

                Card *prev = (Card *)depot.GetFacet(card->stackParent, Facet_Card);
                if (prev) {
                    prev->stackChild = 0;
                }

                card->stackParent = 0;
                break;
            }
            case MsgType_Card_Notify_DragEnd: {
                UID droppedCardUid = msg.uid;
                Card *card = (Card *)depot.GetFacet(droppedCardUid, Facet_Card);
                if (!card) {
                    // Decks aren't cards
                    continue;
                }

                std::unordered_set<UID> stackUids{};

                {
                    DLB_ASSERT(card->stackParent == 0);
                    Card *c = card;
                    while (c) {
                        stackUids.insert(c->uid);
                        c = (Card *)depot.GetFacet(c->stackChild, Facet_Card);
                    }
                }

                float maxDepth = 0;
                Card *topCard = 0;
                for (const Collision &collision : collisionList) {
                    // Check if cursor collision, and resolve out-of-order A/B nonsense
                    UID targetUid = 0;
                    if (droppedCardUid == collision.uidA) {
                        targetUid = collision.uidB;
                    } else if (droppedCardUid == collision.uidB) {
                        targetUid = collision.uidA;
                    } else {
                        continue;
                    }

                    // Don't drop cards on other cards in the same stack (including themselves!)
                    if (stackUids.contains(targetUid)) {
                        continue;
                    }

                    // Check if target is a card
                    Card *targetCard = (Card *)depot.GetFacet(targetUid, Facet_Card);
                    if (targetCard) {
                        // Check if target is a higher card
                        Position *targetPos = (Position *)depot.GetFacet(targetUid, Facet_Position);
                        DLB_ASSERT(targetPos);
                        if (targetPos && targetPos->Depth() > maxDepth) {
                            topCard = targetCard;
                            maxDepth = targetPos->Depth();
                        }
                    }
                }

                if (topCard) {
                    Card *lastChild = topCard;
                    while (lastChild && lastChild->stackChild) {
                        lastChild = (Card *)depot.GetFacet(lastChild->stackChild, Facet_Card);
                    }

                    DLB_ASSERT(lastChild);
                    DLB_ASSERT(lastChild->uid != droppedCardUid);
                    card->stackParent = lastChild->uid;
                    lastChild->stackChild = card->uid;
                }

                break;
            }
            default: break;
        }
    }
}

void CardSystem::Display(Depot &depot, DrawQueue &drawQueue)
{
    for (Deck &deck : depot.deck) {
        Position *position = (Position *)depot.GetFacet(deck.uid, Facet_Position);
        DLB_ASSERT(position);
        if (!position) {
            SDL_LogError(0, "ERROR: Can't draw a deck with no position");
            continue;
        }

        Sprite *sprite = (Sprite *)depot.GetFacet(deck.uid, Facet_Sprite);
        if (!sprite) {
            SDL_LogError(0, "ERROR: Can't draw a deck with no sprite");
            continue;
        }

        rect srcRect = sprite->GetSrcRect();

        rect dstRect{};
        dstRect.x = position->pos.x;
        dstRect.y = position->pos.y - position->pos.z;
        dstRect.w = srcRect.w;
        dstRect.h = srcRect.h;

        float depth = position->pos.y - position->pos.z + position->size.y;
        for (Cursor &cursor : depot.cursor) {
            if (cursor.uidDragSubject == sprite->uid) {
                depth = SCREEN_H * 2.0f;
            }
        }

        DrawCommand drawSprite{};
        drawSprite.uid = sprite->uid;
        drawSprite.color = sprite->color;
        drawSprite.srcRect = srcRect;
        drawSprite.dstRect = dstRect;
        drawSprite.texture = sprite->GetSDLTexture();
        drawSprite.depth = depth;
        drawQueue.push_back(drawSprite);
    }

    for (Card &card : depot.card) {
        vec3 pos{};
        vec2 size{};

#if 1
        Position *position = (Position *)depot.GetFacet(card.uid, Facet_Position);
        pos = position->pos;
        size = position->size;
#else
        int stackDepth = 0;
        Card *c = &card;
        while (c) {
            if (c->stackParent) {
                Card *parent = (Card *)depot.GetFacet(c->stackParent, Facet_Card);
                DLB_ASSERT(parent);
                if (!parent) {
                    SDL_LogError(0, "ERROR: Card has invalid stack parent");
                    break;
                }
                c = parent;
                stackDepth++;
            } else {
                Position *position = (Position *)depot.GetFacet(c->uid, Facet_Position);
                pos = position->pos;
                pos.y += stackDepth * 20.0f;
                size = position->size;
                break;
            }
        }
#endif
        Sprite *sprite = (Sprite *)depot.GetFacet(card.uid, Facet_Sprite);
        if (!sprite) {
            SDL_LogError(0, "ERROR: Can't draw a card with no sprite");
            continue;
        }

        rect srcRect = sprite->GetSrcRect();

        rect dstRect{};
        dstRect.x = pos.x;
        dstRect.y = pos.y - pos.z;
        dstRect.w = srcRect.w;
        dstRect.h = srcRect.h;

        float depth = pos.y - pos.z + size.y;
        for (Cursor &cursor : depot.cursor) {
            Card *c = &card;
            float stackDepth = 0;
            while (c) {
                if (cursor.uidDragSubject == c->uid) {
                    depth = SCREEN_H * 2.0f + stackDepth;
                }
                stackDepth++;
                c = (Card *)depot.GetFacet(c->stackParent, Facet_Card);
            }
        }

        DrawCommand drawSprite{};
        drawSprite.uid = sprite->uid;
        drawSprite.color = sprite->color;
        drawSprite.srcRect = srcRect;
        drawSprite.dstRect = dstRect;
        drawSprite.texture = sprite->GetSDLTexture();
        drawSprite.depth = depth;
        drawQueue.push_back(drawSprite);

#if 0
        Text *text = (Text *)depot.GetFacet(card.uid, Facet_Text);
        if (text) {
            const size_t uidLen = 8;
            char *stackParent = (char *)depot.frameArena.Alloc(uidLen);
            snprintf(stackParent, uidLen, "%u", card.stackParent);
            text->str = stackParent;
        }
#endif
    }

    std::sort(drawQueue.begin(), drawQueue.end());
}