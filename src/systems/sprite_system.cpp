#include "sprite_system.h"
#include "../facets/depot.h"

void SpriteSystem::InitSprite(Depot &depot, Sprite &sprite, vec4 color,
    UID uidSpritesheet, const char *animation)
{
    sprite.color = color;
    sprite.spritesheet = uidSpritesheet;
    sprite.animation = animation;
    sprite.UpdateRect(depot);
}

void SpriteSystem::React(Depot &depot)
{
    size_t size = depot.msgQueue.size();
    for (int i = 0; i < size; i++) {
        Message msg = depot.msgQueue[i];
        Sprite *sprite = (Sprite *)depot.GetFacet(msg.uid, Facet_Sprite);
        if (!sprite) {
            continue;
        }

        switch (msg.type) {
            case MsgType_Sprite_UpdateAnimation:
            {
                Spritesheet *sheet = (Spritesheet *)depot.GetFacet(sprite->spritesheet, Facet_Spritesheet);
                if (sheet) {
                    const char *animName = msg.data.sprite_updateanimation.anim_name;
                    const auto &result = sheet->animations_by_name.find(animName);
                    if (result != sheet->animations_by_name.end()) {
                        sprite->animation = result->first.c_str();
                        sprite->UpdateRect(depot);
                    }
                }
                break;
            }
            default: break;
        }
    }
}

void SpriteSystem::Update(Depot &depot)
{
    // Update animated sprites
    if (depot.Now() - lastAnimAt >= fixedAnimDt) {
        for (Sprite &sprite : depot.sprite) {
            Spritesheet *sheet = (Spritesheet *)depot.GetFacet(sprite.spritesheet, Facet_Spritesheet);
            if (sheet) {
                if (sprite.animation) {
                    const auto &result = sheet->animations_by_name.find(sprite.animation);
                    if (result != sheet->animations_by_name.end()) {
                        Animation &anim = sheet->animations[result->second];
                        sprite.frame = (sprite.frame + 1) % anim.count;
                        sprite.UpdateRect(depot);
                    }
                }
            }
        }
        lastAnimAt = depot.Now();
    }
}