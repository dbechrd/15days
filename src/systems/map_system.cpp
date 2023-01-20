#include "map_system.h"
#include "../facets/depot.h"

void MapSystem::Init(Depot &depot)
{
    //uidEmptyMapSlotSprite = depot.Alloc("empty_map_slot");
    //Sprite *sprite = (Sprite *)depot.AddFacet(uidEmptyMapSlotSprite, Facet_Sprite);
    //SpriteSystem::InitSprite(depot, *sprite, C255(COLOR_WHITE), "sheet_cards", "card_empty_slot");
}

void MapSystem::DrawMapSlot(Depot &depot, DrawQueue &drawQueue, MapSlot &mapSlot, vec3 drawPos)
{
    static std::unordered_map<const char *, UID> mapRoomSprites{};

    UID uidSprite = 0;

    const ResourceDB::MapRoom *mapRoom = depot.resources->map_rooms()->LookupByKey(mapSlot.mapRoomKey);
    if (mapRoom) {
        const auto &result = mapRoomSprites.find(mapSlot.mapRoomKey);
        if (result != mapRoomSprites.end()) {
            uidSprite = result->second;
        }

        if (!uidSprite) {
            uidSprite = depot.Alloc(mapSlot.mapRoomKey);
            depot.AddFacet(uidSprite, Facet_Position);
            Sprite *sprite = (Sprite *)depot.AddFacet(uidSprite, Facet_Sprite);
            const char *spritesheetKey = mapRoom->spritesheet_key()->c_str();
            const char *animationKey = mapRoom->animation_key()->c_str();
            SpriteSystem::InitSprite(depot, *sprite, C255(COLOR_WHITE),
                spritesheetKey,
                animationKey
            );
            mapRoomSprites[mapSlot.mapRoomKey] = uidSprite;
        }
    } else {
        //uidSprite = uidEmptyMapSlotSprite;
        SDL_LogError(0, "Can't draw map slot for invalid map room '%s'", mapSlot.mapRoomKey);
    }

    Sprite *sprite = (Sprite *)depot.GetFacet(uidSprite, Facet_Sprite);
    if (!sprite) {
        SDL_LogError(0, "Can't draw a map room with no sprite");
        return;
    }

    rect srcRect = sprite->GetSrcRect();
    rect dstRect{ drawPos.x, drawPos.y, srcRect.w, srcRect.h };

    // TODO: Check pending drag target is a map slot and highlight it
    //if (card.uid == pendingDragTarget) {
    //    drawSlot.color = C255(COLOR_RED);
    //    drawSlot.outline = true;
    //} else {
    //    drawSlot.color = sprite->color;
    //}

    DrawCommand drawSlot{};
    drawSlot.uid = sprite->uid;
    drawSlot.color = C255(COLOR_WHITE);
    drawSlot.srcRect = srcRect;
    drawSlot.dstRect = dstRect;
    drawSlot.texture = sprite->GetSDLTexture();
    drawSlot.depth = drawPos.y;
    drawQueue.push_back(drawSlot);
}

void MapSystem::Display(Depot &depot, DrawQueue &drawQueue)
{
    for (Map &map : depot.map) {
        Position *position = (Position *)depot.GetFacet(map.uid, Facet_Position);
        if (!position) {
            SDL_LogError(0, "Can't draw a card with no position");
            continue;
        }

        vec3 cursor{ position->pos };
        for (int y = 0; y < MAP_MAX_H; y++) {
            for (int x = 0; x < MAP_MAX_W; x++) {
                DrawMapSlot(depot, drawQueue, map.slots[y][x], cursor);
                cursor.x += 100;
            }
            cursor.x = position->pos.x;
            cursor.y += 150;
        }
    }
}