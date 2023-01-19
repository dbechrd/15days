#pragma once
#include "../common/basic.h"
#include "../common/collision.h"
#include "../common/draw_list.h"
#include "../common/message.h"
#include "../facets/card.h"
#include "../facets/trigger.h"

struct Card_SpawnCardRequest {
    CardType        cardType     {};
    const char *    cardProtoKey {};
    vec3            spawnPos     {};
    TriggerCallback msgCallback  {};
    union {
        struct {
            bool isDeckDraw {};
        } card;
        struct {
            int cardCount {};
        } deck;
    } data {};
};

typedef std::vector<Card_SpawnCardRequest> Card_SpawnCardQueue;

struct CardSystem {
public:
    void PushSpawnDeck(Depot &depot, const char *cardProtoKey, vec3 spawnPos, TriggerCallback msgCallback, int cardCount);
    void PushSpawnCard(Depot &depot, const char *cardProtoKey, vec3 spawnPos, TriggerCallback msgCallback, bool isDeckDraw = false);
    void ProcessQueues(Depot &depot);
    void Update(Depot &depot, const CollisionList &collisionList);
    void Display(Depot &depot, DrawQueue &drawQueue);

private:
    Card_SpawnCardQueue spawnCardQueue{};
    UID pendingDragTarget{};

    Card *FindDragTarget(Depot &depot, const CollisionList &collisionList, struct Card *dragSubject);
    void SpawnCardInternal(Depot &depot, const Card_SpawnCardRequest &spawnCardRequest);
    void UpdateCards(Depot &depot);
    void UpdateStacks(Depot &depot, const CollisionList &collisionList);
};
