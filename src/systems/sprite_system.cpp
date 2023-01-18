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

void SpriteSystem::PushUpdateAnimation(Depot &depot, UID uidSprite,
    const char *spritesheetKey, const char *animationKey, int frame)
{
    Msg_Sprite_UpdateAnimationRequest updateAnimationRequest{};
    updateAnimationRequest.uidSprite = uidSprite;
    updateAnimationRequest.spritesheetKey = spritesheetKey;
    updateAnimationRequest.animationKey = animationKey;
    updateAnimationRequest.frame = frame;
    updateAnimationQueue.push_back(updateAnimationRequest);
}

void SpriteSystem::UpdateAnimationInternal(Depot &depot,
    const Msg_Sprite_UpdateAnimationRequest &updateAnimationRequest)
{
    if (!updateAnimationRequest.spritesheetKey) return;
    if (!updateAnimationRequest.animationKey) return;

    Sprite *sprite = (Sprite *)depot.GetFacet(updateAnimationRequest.uidSprite, Facet_Sprite);
    if (!sprite) return;

    const ResourceDB::Spritesheet *sheet =
        depot.resources->spritesheets()->LookupByKey(sprite->spritesheetKey);
    if (sheet) {
        sprite->spritesheetKey = updateAnimationRequest.spritesheetKey;
        sprite->animationKey = updateAnimationRequest.animationKey;
        if (sprite->animationKey) {
            const ResourceDB::Animation *dbAnimation = sheet->animations()->LookupByKey(sprite->animationKey);
            if (dbAnimation) {
                sprite->frame = CLAMP(updateAnimationRequest.frame, 0, dbAnimation->frame_count() - 1);
            }
        }
        sprite->UpdateRect(depot);
    }
}

void SpriteSystem::ProcessQueues(Depot &depot)
{
    for (const auto &updateAnimationRequest : updateAnimationQueue) {
        UpdateAnimationInternal(depot, updateAnimationRequest);
    }
    updateAnimationQueue.clear();
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