#pragma once
#include "../common/basic.h"
#include "../common/draw_list.h"
#include "../common/message.h"
#include "../facets/sprite.h"

struct Sprite_UpdateAnimationRequest {
    UID          uidSprite      {};
    const char * spritesheetKey {};
    const char * animationKey   {};
    int          frame          {};
};

typedef std::vector<Sprite_UpdateAnimationRequest> Sprite_UpdateAnimationQueue;

struct SpriteSystem {
    // TODO: I feel like someone else maybe should own this, but idk yet
    static void InitSprite(Depot &depot, Sprite &sprite, vec4 color,
        const char *spritesheetKey, const char *animationKey, int frame = 0);

    void PushUpdateAnimation(UID uidSprite, const char *spritesheetKey,
        const char *animationKey, int frame = 0);
    void ProcessQueues(Depot &depot);

    void Update(Depot &depot);

private:
    Sprite_UpdateAnimationQueue updateAnimationQueue{};
    const double fixedAnimDt = 1.0 / 20.0;
    double lastAnimAt = 0;

    void UpdateAnimationInternal(Depot &depot,
        const Sprite_UpdateAnimationRequest &updateAnimationRequest);
};