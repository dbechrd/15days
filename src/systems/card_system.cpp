#include "card_system.h"
#include "../facets/depot.h"

UID CardSystem::CardDragSounds(Depot &depot)
{
    const char *name = "draggable_sounds";

    // Check if already loaded
    if (depot.uidByName.contains(name)) {
        return depot.uidByName[name];
    }

    UID uidDraggableSounds = depot.Alloc(name);
    depot.triggerSystem.Trigger_Audio_PlaySound(depot, uidDraggableSounds, MsgType_Cursor_Notify_DragBegin, "audio/drag_begin.wav");
    depot.triggerSystem.Trigger_Audio_PlaySound(depot, uidDraggableSounds, MsgType_Cursor_Notify_DragEnd, "audio/drag_end.wav");
    return uidDraggableSounds;
}

void CardSystem::DrawCardFromDeck(Depot &depot, UID uidDeck)
{
    Card *card = (Card *)depot.GetFacet(uidDeck, Facet_Card);
    if (!card) {
        return;
    }

    if (card->stackChild) {
        return;
    }

#if 1
    int cardsDrawn = 0;
    while (card->deckCount && !card->stackChild && cardsDrawn < 5) {
#else
    if (card->deckCount) {
#endif
        vec3 spawnPos{};
        Position *position = (Position *)depot.GetFacet(uidDeck, Facet_Position);
        if (position) {
            spawnPos = position->pos;
            spawnPos.y += 50;  // TODO: Rand pop
        }
        Sprite *sprite = (Sprite *)depot.GetFacet(uidDeck, Facet_Sprite);
        if (sprite) {
            // TODO: Search by name or use enum or something smart
            const char *cardProtoKey = 0;
            float rng = dlb_rand32f_range(0, 1);
            if (rng < 0.5f) {
                cardProtoKey = "card_proto_lighter";
            } else if (rng < 0.9f) {
                cardProtoKey = "card_proto_water_bucket";
            } else {
                cardProtoKey = "card_proto_bomb";
            }
            UID uidNewCard = SpawnCard(depot, cardProtoKey, spawnPos, 0.5);

            Body *body = (Body *)depot.GetFacet(uidNewCard, Facet_Body);
            body->impulseBuffer.x = dlb_rand32f_range(0.0f, 1.0f) * (dlb_rand32i_range(0, 1) ? 1.0f : -1.0f);
            body->impulseBuffer.y = dlb_rand32f_range(0.0f, 1.0f) * (dlb_rand32i_range(0, 1) ? 1.0f : -1.0f);
            v3_scalef(v3_normalize(&body->impulseBuffer), dlb_rand32f_range(800.0f, 1200.0f));
            body->jumpBuffer = 12.0f + dlb_rand32f_variance(2.0f);
            depot.renderSystem.Shake(depot, 3.0f, 100.0f, 0.1f);
        } else {
            SDL_Log("Cannot draw from deck with no spritesheet\n");
        }

        card = (Card *)depot.GetFacet(uidDeck, Facet_Card);
        card->deckCount--;
        cardsDrawn++;
    }

    if (!card->deckCount) {
        // TODO: Destroy the deck
        // TODO: Remove "Trigger" as a Facet and go back to std::vector<Trigger> inside of TriggerList??
    }
}

UID CardSystem::SpawnDeck(Depot &depot, const char *cardProtoKey, vec3 spawnPos, int cardCount)
{
    UID uidCard = SpawnCard(depot, cardProtoKey, spawnPos);

    Card *card = (Card *)depot.GetFacet(uidCard, Facet_Card);
    card->cardType = CardType_Deck;
    card->deckCount = cardCount;

    TriggerList *triggerList = (TriggerList *)depot.AddFacet(uidCard, Facet_TriggerList, false);

    Trigger deckDrawTrigger{};
    deckDrawTrigger.trigger = MsgType_Card_DoAction;
    deckDrawTrigger.message.uid = uidCard;
    deckDrawTrigger.message.type = MsgType_Card_Spawn;
    triggerList->triggers.push_back(deckDrawTrigger);

    return uidCard;
}

UID CardSystem::SpawnCard(Depot &depot, const char *cardProtoKey, vec3 spawnPos, float invulnFor)
{
    UID uidCard = depot.Alloc(cardProtoKey, false);

    Position *position = (Position *)depot.AddFacet(uidCard, Facet_Position);
    position->pos = spawnPos;

    Card *card = (Card *)depot.AddFacet(uidCard, Facet_Card);
    card->cardType = CardType_Card;
    card->cardProto = cardProtoKey;
    card->noClickUntil = depot.Now() + invulnFor;

    const ResourceDB::CardProto *cardProto = depot.resources->card_protos()->LookupByKey(cardProtoKey);
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
        return 0;
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

    // TODO: Fix me..
    //depot.triggerSystem.Trigger_Special_RelayAllMessages(depot, uidCard, uidCardProto);
    depot.triggerSystem.Trigger_Special_RelayAllMessages(depot, uidCard, CardDragSounds(depot));

    return uidCard;
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
            Message msgUpdateAnim{};
            msgUpdateAnim.uid = card.uid;
            msgUpdateAnim.type = MsgType_Sprite_UpdateAnimation;
            msgUpdateAnim.data.sprite_updateanimation.animKey = "card_backface";
            depot.msgQueue.push_back(msgUpdateAnim);
        } else if (card.noClickUntil) {
            card.noClickUntil = 0;

            const ResourceDB::CardProto *cardProto = depot.resources->card_protos()->LookupByKey(card.cardProto);
            if (!cardProto) {
                printf("WARN: Can't update a card with no card prototype");
                continue;
            }

            Message msgUpdateAnim{};
            msgUpdateAnim.uid = card.uid;
            msgUpdateAnim.type = MsgType_Sprite_UpdateAnimation;
            msgUpdateAnim.data.sprite_updateanimation.animKey = cardProto->default_animation()->c_str();
            depot.msgQueue.push_back(msgUpdateAnim);

            Message msgTryStack{};
            msgTryStack.type = MsgType_Card_TryToStack;
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
            case MsgType_Cursor_Notify_DragBegin: {
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
            case MsgType_Card_TryToStack: {
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
                    if (targetCard && (targetCard->cardType == card->cardType)) {
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

void CardSystem::React(Depot &depot)
{
    for (int i = 0; i < depot.msgQueue.size(); i++) {
        Message msg = depot.msgQueue[i];

        switch (msg.type) {
            case MsgType_Card_Spawn:
            {
                DrawCardFromDeck(depot, msg.uid);
                break;
            }
            default: break;
        }
    }
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

        rect srcRect = sprite->GetSrcRect();

        rect dstRect{};
        dstRect.x = position->pos.x;
        dstRect.y = position->pos.y - position->pos.z;
        dstRect.w = srcRect.w;
        dstRect.h = srcRect.h;

        float depth = position->pos.y - position->pos.z + position->size.y;
        float stackDepth = 0;
        Card *c = &card;
        while (c) {
            if (cursor.uidDragSubject == c->uid) {
                depth = SCREEN_H * 2.0f + stackDepth;
            }
            stackDepth++;
            c = (Card *)depot.GetFacet(c->stackParent, Facet_Card);
        }

        DrawCommand drawSprite{};
        drawSprite.uid = sprite->uid;
        drawSprite.color = sprite->color;
        if (card.cardType == CardType_Deck && !card.deckCount) {
            drawSprite.color = C255(COLOR_GRAY_5);
        }
        drawSprite.srcRect = srcRect;
        drawSprite.dstRect = dstRect;
        drawSprite.texture = sprite->GetSDLTexture();
        drawSprite.depth = depth;
        drawQueue.push_back(drawSprite);
    }

    std::sort(drawQueue.begin(), drawQueue.end());





    // HACK(dlb): Don't put dis here u dummy, it's the player, not a card
    // NOTE(guy): Okay boomer, but why even have a player in a card game?
    for (Combat &player : depot.combat) {
        Position *position = (Position *)depot.GetFacet(player.uid, Facet_Position);
        vec3 pos = position->pos;
        vec2 size = position->size;

        Sprite *sprite = (Sprite *)depot.GetFacet(player.uid, Facet_Sprite);
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
}