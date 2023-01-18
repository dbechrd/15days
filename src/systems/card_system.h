#pragma once
#include "../common/basic.h"
#include "../common/collision.h"
#include "../common/draw_list.h"
#include "../common/message.h"

typedef std::vector<Msg_Card_SpawnCardRequest> Card_SpawnCardQueue;

struct CardSystem {
public:
    void PushSpawnDeck(Depot &depot, const char *cardProtoKey, vec3 spawnPos, int cardCount);
    void PushSpawnCard(Depot &depot, const char *cardProtoKey, vec3 spawnPos, bool isDeckDraw = false);
    void ProcessQueues(Depot &depot);

    void UpdateCards(Depot &depot);
    void UpdateStacks(Depot &depot, const CollisionList &collisionList);

    void Display(Depot &depot, DrawQueue &drawQueue);

private:
    Card_SpawnCardQueue spawnCardQueue{};
    UID pendingDragTarget{};

    void SpawnCardInternal(Depot &depot, const Msg_Card_SpawnCardRequest &spawnCardRequest);
    struct Card *FindDragTarget(Depot &depot, const CollisionList &collisionList, struct Card *dragSubject);
};
