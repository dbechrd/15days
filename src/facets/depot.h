#pragma once
#include "../common/basic.h"
#include "../common/game_state.h"

#include "attach.h"
#include "body.h"
#include "combat.h"
#include "keymap.h"
#include "position.h"
#include "sprite.h"
#include "trigger.h"

#include <vector>
#include <unordered_map>

//-----------------------------------------------------------------------------
// Facets
//-----------------------------------------------------------------------------

// Location of mouse cursor in screen coords (and world coords if relevant)
struct InputCursor : public Facet {
    vec2  screen      {};  // position of cursor in screen coordinates
    // TODO: CameraSystem should probably populate this?
    bool  worldActive {};  // if false, world position is undefined
    vec2  world       {};  // position of cursor in world coordinates
};

//-----------------------------------------------------------------------------
// Facet Depot
//-----------------------------------------------------------------------------

struct Depot {
    // Map of pool indices for each active UID, per facet type
    std::unordered_map<UID, uint32_t> indexByUid[Facet_Count]{};

    //// List of active UIDs for each facet type
    //std::vector<UID> uidsByType[Facet_Count]{};

    // Dense facet data arrays
    std::vector<Attach>       attach      {};
    std::vector<Body>         body        {};
    std::vector<Combat>       combat      {};
    std::vector<InputCursor>  inputCursor {};  // could be useful for multiplayer (net or local)
    std::vector<Keymap>       keymap      {};
    std::vector<Position>     position    {};
    std::vector<Sprite>       sprite      {};
    std::vector<Trigger>      trigger     {};

    UID Alloc(void);
    void *AddFacet(UID uid, FacetType type);
    void *GetFacet(UID uid, FacetType type);

private:
    UID nextUid{1};
};

//-----------------------------------------------------------------------------
// Depot System
//-----------------------------------------------------------------------------

struct DepotSystem {
    void Init(GameState state)
    {
        gameState = state;
        gameStatePending = gameState;
    }

    void BeginFrame(void)
    {
        // TODO: Other state change stuff if these differ?
        gameState = gameStatePending;
    }

    Depot &ForState(GameState state)
    {
        return depots[state];
    }

    Depot &Current(void)
    {
        return depots[gameState];
    }

    void TransitionTo(GameState state)
    {
        gameStatePending = state;
    }

private:
    Depot     depots           [GameState_Count]{};
    GameState gameState        {};
    GameState gameStatePending {};  // will change at the start of the next frame
};

//-----------------------------------------------------------------------------
