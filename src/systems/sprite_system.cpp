#include "sprite_system.h"
#include "../facets/depot.h"

void SpriteSystem::InitSprite(Depot &depot, Sprite &sprite, vec4 color,
    const char *spritesheetKey, const char *animationKey)
{
    sprite.color = color;
    sprite.spritesheetKey = spritesheetKey;
    sprite.animationKey = animationKey;
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
                const ResourceDB::Spritesheet *sheet =
                    depot.resources->spritesheets()->LookupByKey(sprite->spritesheetKey);
                if (sheet) {
                    sprite->animationKey = msg.data.sprite_updateanimation.animKey;
                    sprite->UpdateRect(depot);
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
            const ResourceDB::Spritesheet *sheet =
                depot.resources->spritesheets()->LookupByKey(sprite.spritesheetKey);
            if (sheet) {
                if (sprite.animationKey) {
                    const ResourceDB::Animation *anim =
                        sheet->animations()->LookupByKey(sprite.animationKey);
                    const int frameCount = anim->frame_count();
                    if (anim && frameCount) {
                        sprite.frame = (sprite.frame + 1) % frameCount;
                        sprite.UpdateRect(depot);
                    }
                }
            }
        }
        lastAnimAt = depot.Now();
    }
}