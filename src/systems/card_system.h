#pragma once
#include "../common/basic.h"
#include "../common/collision.h"
#include "../common/draw_list.h"

struct CardSystem {
    UID PrototypeCard(Depot &depot, const char *name, UID uidMaterialProto, UID uidEffectList, UID spritesheet, const char *animation);
    UID SpawnDeck(Depot &depot, vec3 pos, UID spritesheet, const char *animation);
    UID SpawnCard(Depot &depot, UID uidCardProto, vec3 spawnPos, float invulnFor = 0);

    void React(Depot &depot);
    void UpdateCards(Depot &depot);
    void UpdateStacks(Depot &depot, const CollisionList &collisionList);
    void Display(Depot &depot, DrawQueue &drawQueue);

private:
    UID CardDragSounds(Depot &depot);
    void DrawCardFromDeck(Depot &depot, UID uidDeck);
};
