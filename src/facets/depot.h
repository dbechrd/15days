#pragma once
#include "../common/arena.h"
#include "../common/basic.h"
#include "../common/game_state.h"
#include "../systems/audio_system.h"
#include "../systems/card_system.h"
#include "../systems/combat_system.h"
#include "../systems/event_system_sdl.h"
#include "../systems/fps_counter_system.h"
#include "../systems/input_system.h"
#include "../systems/movement_system.h"
#include "../systems/physics_system.h"
#include "../systems/render_system.h"
#include "../systems/sprite_system.h"
#include "../systems/text_system.h"
#include "../systems/trigger_system.h"

#include "attach.h"
#include "body.h"
#include "combat.h"
#include "cursor.h"
#include "fps_counter.h"
#include "keymap.h"
#include "position.h"
#include "sound.h"
#include "sprite.h"
#include "text.h"
#include "trigger.h"

#include <vector>
#include <unordered_map>

struct Depot {
    // Map of pool indices for each active UID, per facet type
    std::unordered_map<UID, uint32_t> indexByUid[Facet_Count]{};

    //// List of active UIDs for each facet type
    //std::vector<UID> uidsByType[Facet_Count]{};

    // Dense facet data arrays
    std::vector<Attach>      attach      {};
    std::vector<Body>        body        {};
    std::vector<Combat>      combat      {};
    std::vector<Cursor>      cursor      {};
    std::vector<FpsCounter>  fpsCounter  {};
    std::vector<Keymap>      keymap      {};
    std::vector<Position>    position    {};
    std::vector<Sound>       sound       {};
    std::vector<Sprite>      sprite      {};
    std::vector<Text>        text        {};
    std::vector<Trigger>     trigger     {};
    std::vector<TriggerList> triggerList {};

    MsgQueue msgQueue{};
    Arena frameArena{};

    // There aren't really the same kind of "system", maybe use a different word
    EventSystemSDL eventSystemSDL {};
    InputSystem    inputSystem    {};

    // These are probably order-dependent
    FpsCounterSystem fpsCounterSystem {};
    CardSystem       cardSystem       {};
    MovementSystem   movementSystem   {};
    PhysicsSystem    physicsSystem    {};
    CombatSystem     combatSystem     {};
    SpriteSystem     spriteSystem     {};
    TriggerSystem    triggerSystem    {};  // might react to any gameplay system
    AudioSystem      audioSystem      {};  // might react to triggers
    TextSystem       textSystem       {};  // last because it might render debug UI for any system
    RenderSystem     renderSystem     {};

    UID Alloc(void);
    void *AddFacet(UID uid, FacetType type, bool canExist = false);
    void *GetFacet(UID uid, FacetType type);

    void Init(GameState state)
    {
        gameState = state;
        gameStatePending = gameState;
        frameArena.Init(KB(16));
    }

    void Destroy(void)
    {
        // TODO: Make Destroy() placeholder for other systems that don't
        // currently need to destroy things
        renderSystem.DestroyDepot(*this);
        audioSystem.DestroyDepot(*this);
        renderSystem.Destroy();
        audioSystem.Destroy();

        frameArena.Destroy();
    }

    void BeginFrame(void)
    {
        // TODO: Other state change stuff if these differ?
        gameState = gameStatePending;

        Message msgBeginFrame{};
        msgBeginFrame.type = MsgType_Render_FrameBegin;
        msgBeginFrame.data.render_framebegin.realDtSmooth = realDtSmooth;
        msgQueue.push_back(msgBeginFrame);
    }

    void EndFrame(void)
    {
        msgQueue.clear();
        frameArena.Clear();
    }

    void TransitionTo(GameState state)
    {
        gameStatePending = state;
    }

    void Run(void);

private:
    GameState gameState{};
    GameState gameStatePending{};  // will change at the start of the next frame
    UID nextUid{1};

    uint64_t frame{};
    double physicsAccum{};
    double nowPrev{};
    double now{};
    const double fixedDt = 1.0 / 60.0;
    double realDtSmooth = fixedDt;

    InputQueue inputQueue{};  // raw input (abstracted from platform)
};
