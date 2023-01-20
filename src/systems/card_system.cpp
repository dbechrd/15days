#include "card_system.h"
#include "../facets/depot.h"

void deck_try_draw_card(Depot &depot, Card *card)
{
    if (card->cardType != CardType_Deck) {
        return;
    }
    if (card->stackChild) {
        return;
    }

    vec3 spawnPos{};
    Position *position = (Position *)depot.GetFacet(card->uid, Facet_Position);
    if (position) {
        spawnPos = position->pos;
        spawnPos.y += 50;  // TODO: Rand pop
    }

#if 1
    int cardsDrawn = 0;
    while (card->data.deck.cardCount && !card->stackChild && cardsDrawn < 5) {
#else
    if (card->data.deck.cardCount) {
#endif
        // TODO: Deck should own list of protos/chances
        const char *cardProtoKey = 0;
        float rng = dlb_rand32f_range(0, 1);
        if (rng < 0.5f) {
            cardProtoKey = "card_proto_lighter";
        } else if (rng < 0.9f) {
            cardProtoKey = "card_proto_water_bucket";
        } else {
            cardProtoKey = "card_proto_bomb";
        }

        depot.cardSystem.PushSpawnCard(depot, cardProtoKey, spawnPos, 0, true);
        depot.renderSystem.PushShake(depot, 3.0f, 100.0f, 0.1f);

        card->data.deck.cardCount--;
        cardsDrawn++;
    }

    if (!card->data.deck.cardCount) {
        // TODO: Destroy the deck
        // TODO: Remove "Trigger" as a Facet and go back to std::vector<Trigger> inside of TriggerList??
    }
}

void card_callback(Depot &depot, const Message &msg, const Trigger &trigger, void *userData)
{
    Card *card = (Card *)depot.GetFacet(msg.uid, Facet_Card);
    if (!card) {
        return;
    }

    const ResourceDB::CardProto *cardProto = depot.resources->card_protos()->LookupByKey(card->cardProto);
    const char *dragBeginSoundKey = (cardProto->drag_begin_sound_key()) ? cardProto->drag_begin_sound_key()->c_str() : 0;
    const char *dragUpdateSoundKey = (cardProto->drag_update_sound_key()) ? cardProto->drag_update_sound_key()->c_str() : 0;
    const char *dragEndSoundKey = (cardProto->drag_end_sound_key()) ? cardProto->drag_end_sound_key()->c_str() : 0;

    switch (msg.type) {
        case MsgType_Cursor_Notify_DragBegin:
        {
            depot.audioSystem.PushPlaySound(depot, "sfx_drag_begin");
            depot.audioSystem.PushPlaySound(depot, dragBeginSoundKey);

            // Break from parent stack
            Card *parent = (Card *)depot.GetFacet(card->stackParent, Facet_Card);
            if (parent) parent->stackChild = 0;
            card->stackParent = 0;
            card->wantsToStack = false;

            break;
        }
        case MsgType_Cursor_Notify_DragUpdate:
        {
            depot.audioSystem.PushPlaySound(depot, dragUpdateSoundKey);

            vec2 cursorPos = msg.data.cursor_dragevent.currentPos;
            vec2 dragOffset = msg.data.cursor_dragevent.subjectOffset;

            Position *pos = (Position *)depot.GetFacet(card->uid, Facet_Position);
            if (pos) {
                pos->pos.x = cursorPos.x - dragOffset.x;
                pos->pos.y = cursorPos.y - dragOffset.y;
            } else {
                SDL_LogError(0, "Cannot drag card with no position");
            }
            break;
        }
        case MsgType_Cursor_Notify_DragEnd:
        {
            depot.audioSystem.PushStopSound(depot, dragUpdateSoundKey);
            depot.audioSystem.PushPlaySound(depot, "sfx_drag_end");
            depot.audioSystem.PushPlaySound(depot, dragEndSoundKey, true);

            // If drag was tiny, treat as click
            const vec2 dragOffset = msg.data.cursor_dragevent.dragOffset;
            const float tinyDrag = 5.0f;
            if (fabs(dragOffset.x) < tinyDrag && fabs(dragOffset.y) < tinyDrag) {
                if (card->cardType == CardType_Deck) {
                    deck_try_draw_card(depot, card);
                }
            } else {
                // Maybe this should happen during tiny drags too?
                card->wantsToStack = true;
            }

            break;
        }
        default: break;
    }
}

Card *CardSystem::FindDragTarget(Depot &depot, const CollisionList &collisionList, Card *dragSubject)
{
    if (!dragSubject) {
        return 0;
    }

    std::unordered_set<UID> stackUids{};

    {
        DLB_ASSERT(dragSubject->stackParent == 0);
        Card *c = dragSubject;
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
        if (dragSubject->uid == collision.uidA) {
            targetUid = collision.uidB;
        } else if (dragSubject->uid == collision.uidB) {
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
        if (targetCard && (targetCard->cardType == dragSubject->cardType)) {
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
        DLB_ASSERT(lastChild->uid != dragSubject->uid);
        return lastChild;
    }

    return 0;
}

void CardSystem::PushSpawnDeck(Depot &depot, const char *cardProtoKey, vec3 spawnPos, TriggerCallback msgCallback, int cardCount)
{
    Card_SpawnCardRequest spawnCardRequest{};
    spawnCardRequest.cardType = CardType_Deck;
    spawnCardRequest.cardProtoKey = cardProtoKey;
    spawnCardRequest.spawnPos = spawnPos;
    spawnCardRequest.msgCallback = msgCallback;
    spawnCardRequest.data.deck.cardCount = cardCount;
    spawnCardQueue.push_back(spawnCardRequest);
}

void CardSystem::PushSpawnCard(Depot &depot, const char *cardProtoKey, vec3 spawnPos, TriggerCallback msgCallback, bool isDeckDraw)
{
    Card_SpawnCardRequest spawnCardRequest{};
    spawnCardRequest.cardType = CardType_Card;
    spawnCardRequest.cardProtoKey = cardProtoKey;
    spawnCardRequest.spawnPos = spawnPos;
    spawnCardRequest.msgCallback = msgCallback;
    spawnCardRequest.data.card.isDeckDraw = isDeckDraw;
    spawnCardQueue.push_back(spawnCardRequest);
}

void CardSystem::SpawnCardInternal(Depot &depot, const Card_SpawnCardRequest &spawnCardRequest)
{
    UID uidCard = depot.Alloc(spawnCardRequest.cardProtoKey, false);

    Position *position = (Position *)depot.AddFacet(uidCard, Facet_Position);
    position->pos = spawnCardRequest.spawnPos;

    Card *card = (Card *)depot.AddFacet(uidCard, Facet_Card);
    card->cardType = spawnCardRequest.cardType;
    card->cardProto = spawnCardRequest.cardProtoKey;

    const ResourceDB::CardProto *cardProto =
        depot.resources->card_protos()->LookupByKey(spawnCardRequest.cardProtoKey);
    if (cardProto->material_proto()) {
        Material *material = (Material *)depot.AddFacet(uidCard, Facet_Material);
        material->materialProtoKey = cardProto->material_proto()->c_str();
    }

    const char *sheetKey = cardProto->spritesheet()->c_str();
    const ResourceDB::Spritesheet *sheet = depot.resources->spritesheets()->LookupByKey(sheetKey);
    if (sheet) {
        Sprite *sprite = (Sprite *)depot.AddFacet(uidCard, Facet_Sprite);
        SpriteSystem::InitSprite(depot, *sprite, C255(COLOR_WHITE),
            cardProto->spritesheet()->c_str(), cardProto->default_animation()->c_str());
    } else {
        DLB_ASSERT(!"no sheet");
        printf("Failed to find sheet for card\n");
        return;
    }

    Body *body = (Body *)depot.AddFacet(uidCard, Facet_Body);
    body->gravity = -50.0f;
    body->friction = 0.001f;
    //body->drag = 0.001f;
    body->drag = 0.05f;
    body->restitution = 0.0f;
    body->jumpImpulse = 800.0f;
    body->speed = 20.0f;
    body->runMult = 2.0f;
    float mass = 1.0f;
    body->invMass = 1.0f / mass;

    switch (spawnCardRequest.cardType) {
        case CardType_Card:
        {
            if (spawnCardRequest.data.card.isDeckDraw) {
                card->noClickUntil = depot.Now() + 0.5f;

                body->impulseBuffer.x = dlb_rand32f_range(0.0f, 1.0f) * (dlb_rand32i_range(0, 1) ? 1.0f : -1.0f);
                body->impulseBuffer.y = dlb_rand32f_range(0.0f, 1.0f) * (dlb_rand32i_range(0, 1) ? 1.0f : -1.0f);
                v3_scalef(v3_normalize(&body->impulseBuffer), dlb_rand32f_range(800.0f, 1200.0f));
                body->jumpBuffer = 12.0f + dlb_rand32f_variance(2.0f);
            }
            break;
        }
        case CardType_Deck:
        {
            card->data.deck.cardCount = spawnCardRequest.data.deck.cardCount;
            break;
        }
    }

#if 0
    Text *debugText = (Text *)depot.AddFacet(uidCard, Facet_Text);
    debugText->font = depot.textSystem.LoadFont(depot, "font/OpenSans-Bold.ttf", 16);
    // TODO: Use card prototype desc instead once that's in DB file
    //debugText->str = depot.nameByUid[uidCardProto].c_str();
    if (cardProto->default_animation()) {
        const char *defaultAnimKey = cardProto->default_animation()->c_str();
        const ResourceDB::Animation *anim = sheet->animations()->LookupByKey(defaultAnimKey);
        if (anim) {
            debugText->str = anim->desc()->c_str();
        }
    }
    debugText->align = TextAlign_VBottom_HCenter;
    debugText->color = C255(COLOR_WHITE);
    debugText->offset.x += 100;
    debugText->offset.y = 0;
#endif

    depot.triggerSystem.Trigger_Special_RelayAllMessages(depot, uidCard, 0, card_callback);
    depot.triggerSystem.Trigger_Special_RelayAllMessages(depot, uidCard, 0, spawnCardRequest.msgCallback);
}

void CardSystem::ProcessQueues(Depot &depot)
{
    for (const auto &spawnCardRequest : spawnCardQueue) {
        SpawnCardInternal(depot, spawnCardRequest);
    }
    spawnCardQueue.clear();
}

void CardSystem::UpdateStacks(Depot &depot, const CollisionList &collisionList)
{
    DLB_ASSERT(depot.cursor.size() == 1);
    pendingDragTarget = 0;

    UID uidCursorDragSubject = depot.cursor.front().uidDragSubject;

    for (Card &card : depot.card) {
        if (card.uid == uidCursorDragSubject) {
            Card *dragTarget = FindDragTarget(depot, collisionList, &card);
            if (dragTarget) {
                pendingDragTarget = dragTarget->uid;
            }
        } else if (card.wantsToStack) {
#if FDOV_ENABLE_CARD_STACKING
            Card *dragTarget = FindDragTarget(depot, collisionList, &card);
            if (dragTarget) {
                card.stackParent = dragTarget->uid;
                dragTarget->stackChild = card.uid;
            }
#endif
            card.wantsToStack = false;
        }
    }
}

void CardSystem::UpdateCards(Depot &depot)
{
    for (Card &card : depot.card) {
        if (card.stackParent) {
            Position *position = (Position *)depot.GetFacet(card.uid, Facet_Position);
            DLB_ASSERT(position);

            Position *parentPos = (Position *)depot.GetFacet(card.stackParent, Facet_Position);
            DLB_ASSERT(parentPos);

            vec3 targetPos = parentPos->pos;
            targetPos.y += 22.0f;

            const float lerpFac = 1.0f - powf(1.0f - 0.5f, depot.RealDt() * 60.0f);
            position->pos.x = LERP(position->pos.x, targetPos.x, lerpFac);
            position->pos.y = LERP(position->pos.y, targetPos.y, lerpFac);
            position->pos.z = LERP(position->pos.z, targetPos.z, lerpFac);
        }

        if (card.noClickUntil > depot.Now()) {
            depot.spriteSystem.PushUpdateAnimation(depot, card.uid,
                "sheet_cards", "card_backface");
        } else if (card.noClickUntil) {
            card.noClickUntil = 0;

            const ResourceDB::CardProto *cardProto =
                depot.resources->card_protos()->LookupByKey(card.cardProto);
            if (!cardProto) {
                printf("WARN: Can't update a card with no card prototype");
                continue;
            }

            depot.spriteSystem.PushUpdateAnimation(depot, card.uid,
                cardProto->spritesheet()->c_str(),
                cardProto->default_animation()->c_str());

            card.wantsToStack = true;
        }
    }
}

void CardSystem::Update(Depot &depot, const CollisionList &collisionList)
{
    UpdateStacks(depot, collisionList);
    UpdateCards(depot);
}

void CardSystem::Display(Depot &depot, DrawQueue &drawQueue)
{
    const Cursor &cursor = depot.cursor.front();

    for (Card &card : depot.card) {
        Position *position = (Position *)depot.GetFacet(card.uid, Facet_Position);
        if (!position) {
            SDL_LogError(0, "Can't draw a card with no position");
            continue;
        }

        Sprite *sprite = (Sprite *)depot.GetFacet(card.uid, Facet_Sprite);
        if (!sprite) {
            SDL_LogError(0, "Can't draw a card with no sprite");
            continue;
        }

        float depth = position->pos.y - position->pos.z + position->size.y;
        vec2 dragOffset{ 0.0f, 0.0f };

        {
            float stackDepth = 0;
            Card *c = &card;
            while (c) {
                if (cursor.uidDragSubject == c->uid) {
                    depth = SCREEN_H * 2.0f + stackDepth;
                    dragOffset.x += 4.0f;
                    dragOffset.y += 6.0f;
                    break;
                }
                stackDepth++;
                c = (Card *)depot.GetFacet(c->stackParent, Facet_Card);
            }
        }

        rect srcRect = sprite->GetSrcRect();

        if (!v2_iszero(&dragOffset)) {
            const vec2 shadowOffset{ 0.0f, 2.0f };
            rect dstRectShadow {
                shadowOffset.x + position->pos.x,
                shadowOffset.y + position->pos.y - position->pos.z,
                srcRect.w,
                srcRect.h
            };

            DrawCommand drawShadow{};
            drawShadow.uid = sprite->uid;
            drawShadow.color = C255(((vec4){ 0.1f, 0.1f, 0.1f, 0.7f }));
            drawShadow.srcRect = srcRect;
            drawShadow.dstRect = dstRectShadow;
            drawShadow.depth = depth - 0.01f;
            drawQueue.push_back(drawShadow);
        }

        rect dstRectCard{
            position->pos.x + dragOffset.x,
            position->pos.y - position->pos.z - dragOffset.y,
            srcRect.w,
            srcRect.h
        };

        DrawCommand drawSprite{};
        drawSprite.uid = sprite->uid;
        if (card.uid == pendingDragTarget) {
            drawSprite.color = C255(COLOR_RED);
            drawSprite.outline = true;
        } else if (card.cardType == CardType_Deck && !card.data.deck.cardCount) {
            drawSprite.color = C255(COLOR_GRAY_5);
        } else {
            drawSprite.color = sprite->color;
        }

        if (card.wantsToStack) {
            drawSprite.color = C255(COLOR_MAGENTA);
        }

        drawSprite.srcRect = srcRect;
        drawSprite.dstRect = dstRectCard;
        drawSprite.texture = sprite->GetSDLTexture();
        drawSprite.depth = depth;
        drawQueue.push_back(drawSprite);
    }
}