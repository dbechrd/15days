#pragma once
#include "fbg/ResourceDB_generated.h"
#include "fbg/SaveFile_generated.h"

#include "../common/arena.h"
#include "../common/basic.h"
#include "../common/game_state.h"

#include "../systems/audio_system.h"
#include "../systems/card_system.h"
#include "../systems/combat_system.h"
#include "../systems/collision_system.h"
#include "../systems/cursor_system.h"
#include "../systems/effect_system.h"
#include "../systems/event_system_sdl.h"
#include "../systems/input_system.h"
#include "../systems/histogram_system.h"
#include "../systems/movement_system.h"
#include "../systems/physics_system.h"
#include "../systems/render_system.h"
#include "../systems/sprite_system.h"
#include "../systems/text_system.h"
#include "../systems/trigger_system.h"

#include "attach.h"
#include "body.h"
#include "card.h"
#include "combat.h"
#include "cursor.h"
#include "deck.h"
#include "font.h"
#include "fps_counter.h"
#include "histogram.h"
#include "keymap.h"
#include "material.h"
#include "position.h"
#include "sound.h"
#include "sprite.h"
#include "text.h"
#include "texture.h"
#include "trigger.h"

struct Depot {
    const ResourceDB::Root *resources{};

    // Map of pool indices for each active facet; by UID, per type
    std::unordered_map<std::string, UID>      uidByName   {};
    std::unordered_map<UID, std::string>      nameByUid   {};
    std::unordered_map<UID, uint32_t>         indexByUid  [Facet_Count]{};
    // TODO: Is this a good idea? Should it be sparse? Does that defeat the purpose?
    // Do I even need it anymore after memoizing bboxes?
    std::array<std::bitset<32>, 8192>         bitmapByUid {};
    //dlb_hash indexByUid[Facet_Count]{};
    //std::unordered_map<std::string, uint32_t> indexByName [Facet_Count]{};

    // Dense facet data arrays
    std::vector<Attach>        attach        {};
    std::vector<Body>          body          {};
    std::vector<Card>          card          {};
    std::vector<Combat>        combat        {};
    std::vector<Cursor>        cursor        {};
    std::vector<Deck>          deck          {};
    std::vector<Font>          font          {};
    std::vector<FpsCounter>    fpsCounter    {};
    std::vector<Histogram>     histogram     {};
    std::vector<Keymap>        keymap        {};
    std::vector<Material>      material      {};
    std::vector<Position>      position      {};
    std::vector<Sound>         sound         {};
    std::vector<Sprite>        sprite        {};
    std::vector<Text>          text          {};
    std::vector<Texture>       texture       {};
    std::vector<TriggerList>   triggerList   {};

    MsgQueue msgQueue{};
    Arena frameArena{};
    Arena resourceArena{};  // for e.g. Wav data

    // There aren't really the same kind of "system", maybe use a different word
    EventSystemSDL eventSystemSDL {};
    InputSystem    inputSystem    {};

    // These are order-dependent, see Depot::Run() in depot.cpp
    AudioSystem      audioSystem      {};  // might react to triggers
    CardSystem       cardSystem       {};
    CollisionSystem  collisionSystem  {};
    CombatSystem     combatSystem     {};
    CursorSystem     cursorSystem     {};
    EffectSystem     effectSystem     {};
    HistogramSystem  histogramSystem  {};
    MovementSystem   movementSystem   {};
    PhysicsSystem    physicsSystem    {};
    RenderSystem     renderSystem     {};
    SpriteSystem     spriteSystem     {};
    TextSystem       textSystem       {};  // last because it might render debug UI for any system
    TriggerSystem    triggerSystem    {};  // might react to any gameplay system

    UID Alloc(const char *name, bool unique = true);
    void *AddFacet(UID uid, FacetType type, bool warnDupe = true);
    void *GetFacet(UID uid, FacetType type);
    void *GetFacetByName(const char *name, FacetType type);

    void Init(GameState state)
    {
        gameState = state;
        gameStatePending = gameState;
        frameArena.Init(KB(32));
        resourceArena.Init(KB(4));

        //for (int i = 0; i < Facet_Count; i++) {
        //    dlb_hash_init(&indexByUid[i], DLB_HASH_INT, "indexByUid", 256);
        //}
    }

    void Destroy(void)
    {
        // TODO: Make Destroy() placeholder for other systems that don't
        // currently need to destroy things
        renderSystem.DestroyDepot(*this);
        audioSystem.DestroyDepot(*this);
        renderSystem.Destroy();
        audioSystem.Destroy();

        // TODO: FontSystem?
        for (Font &font : font) {
            TTF_CloseFont(font.ttf_font);
        }

        frameArena.Destroy();
        resourceArena.Destroy();

        //for (int i = 0; i < Facet_Count; i++) {
        //    dlb_hash_free(&indexByUid[i]);
        //}
    }

    void BeginFrame(void)
    {
        // TODO: Other state change stuff if these differ?
        gameState = gameStatePending;

        Message msgBeginFrame{};
        msgBeginFrame.type = MsgType_Render_FrameBegin;
        msgQueue.push_back(msgBeginFrame);
    }

    void EndFrame(void)
    {
        collisionList.clear();
        msgQueue.clear();
        frameArena.Reset();
        hashesLastFrame = hashesThisFrame;
        hashesThisFrame = 0;
    }

    void TransitionTo(GameState state)
    {
        gameStatePending = state;
    }

    void Run(void);

    inline double Now(void) {
        return now;
    }

    inline double Dt(void) {
        return fixedDt;
    }

    inline double RealDt(void) {
        return realDt;
    }

    inline double DtSmooth(void) {
        return realDtSmooth;
    }

    inline double Fps(void) {
        return 1.0f / Dt();
    }

    inline double FpsSmooth(void) {
        return 1.0f / DtSmooth();
    }

    inline size_t Hashes(void) {
        return hashesLastFrame;
    }

private:
    GameState gameState{};
    GameState gameStatePending{};  // will change at the start of the next frame
    UID nextUid{1};

    size_t hashesLastFrame{};
    size_t hashesThisFrame{};
    uint64_t frame{};
    double physicsAccum{};
    double nowPrev{};
    double now{};
    const double fixedDt = 1.0 / 60.0;
    double realDt{};
    double realDtSmooth = fixedDt;

    InputQueue inputQueue{};  // raw input (abstracted from platform)
    CollisionList collisionList{};
};
