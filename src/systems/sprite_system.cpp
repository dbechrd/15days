#include "sprite_system.h"
#include "../facets/depot.h"

void SpriteSystem::InitSprite(Depot &depot, Sprite &sprite, vec4 color, UID uidSpritesheet)
{
    sprite.color = color;
    sprite.SetSpritesheet(depot, uidSpritesheet);
}

void SpriteSystem::Update(double now, Depot &depot)
{
    // TODO: Update sprite states based on game events (e.g. combat messages)
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
                sprite->SetAnimIndex(depot, msg.data.sprite_updateanimation.animation);
                break;
            }
            default: break;
        }
    }

    // Update animated sprites
    if (now - lastAnimAt >= fixedAnimDt) {
        for (Sprite &sprite : depot.sprite) {
            Spritesheet *sheet = (Spritesheet *)depot.GetFacet(sprite.GetSpritesheet(), Facet_Spritesheet);
            if (sheet) {
                Animation &anim = sheet->animations[sprite.GetAnimIndex()];
                sprite.SetAnimFrame(depot, (sprite.GetAnimFrame() + 1) % anim.count);
            }
        }
        lastAnimAt = now;
    }
}