#pragma once
#include "../common/basic.h"
#include "../common/collision.h"
#include "../common/draw_list.h"

struct CardSystem {
private:
    UID CardDragSounds(Depot &depot);
    void DrawCardFromDeck(Depot &depot, UID uidDeck);

public:
    UID SpawnDeck(Depot &depot, const char *cardProtoKey, vec3 spawnPos, int cardCount);
    UID SpawnCard(Depot &depot, const char *cardProtoKey, vec3 spawnPos, float invulnFor = 0);

    void UpdateCards(Depot &depot);
    void UpdateStacks(Depot &depot, const CollisionList &collisionList);

    void React(Depot &depot);
    void Display(Depot &depot, DrawQueue &drawQueue);
};
