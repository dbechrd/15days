#include "depot.h"
#include "../common/clock.h"

UID Depot::Alloc(const char *name, bool unique)
{
    if (!name) {
        name = "unnamed";
        unique = false;
    }

    UID uid = nextUid;
    nextUid++;

    std::string key{ name };
    if (!unique) {
        key += ":" + std::to_string(uid);
    }

    if (uidByName.contains(key)) {
        printf("ERROR: Unique name '%s' is already in use by UID %u\n", key.c_str(), uidByName[key]);
        DLB_ASSERT(!"Expected unique name");
        return 0;
    }

    uidByName[key] = uid;
    nameByUid[uid] = key;

    printf("[%u] %s\n", uid, key.c_str());

    return uid;
}

void *Depot::AddFacet(UID uid, FacetType type, bool warnDupe)
{
    void *existingFacet = GetFacet(uid, type);
    if (existingFacet) {
        if (warnDupe) {
            printf("WARN: AddFacet called more than once for same uid/type pair: %u/%d.\n", uid, type);
        }
        return existingFacet;
    }

#define EMPLACE(label, pool) \
    case label: { \
        index = pool.size(); \
        facet = &pool.emplace_back(); \
        break; \
    }

    size_t index = 0;
    Facet *facet = 0;
    switch (type) {
        EMPLACE(Facet_Attach,        attach);
        EMPLACE(Facet_Body,          body);
        EMPLACE(Facet_Card,          card);
        EMPLACE(Facet_CardProto,     cardProto);
        EMPLACE(Facet_Combat,        combat);
        EMPLACE(Facet_Cursor,        cursor);
        EMPLACE(Facet_Deck,          deck);
        EMPLACE(Facet_EffectList,    effectList);
        EMPLACE(Facet_Font,          font);
        EMPLACE(Facet_FpsCounter,    fpsCounter);
        EMPLACE(Facet_Histogram,     histogram);
        EMPLACE(Facet_Keymap,        keymap);
        EMPLACE(Facet_Material,      material);
        EMPLACE(Facet_MaterialProto, materialProto);
        EMPLACE(Facet_Position,      position);
        EMPLACE(Facet_Sound,         sound);
        EMPLACE(Facet_Sprite,        sprite);
        EMPLACE(Facet_Spritesheet,   spritesheet);
        EMPLACE(Facet_Text,          text);
        EMPLACE(Facet_Texture,       texture);
        EMPLACE(Facet_TriggerList,   triggerList);
        default: assert(!"what is that, mate?");
    }

#undef EMPLACE

    if (!facet) {
        // TODO: Log this instead of printing
        printf("FATAL ERROR: Failed to allocate facet type %d for entity %u\n", type, uid);
        fflush(stdout);
        assert(facet);
        exit(-1);  // fatal error
    }

    facet->uid = uid;
    facet->f_type = type;
    indexByUid[type][uid] = index;
    //bitmapByUid[uid].set(type);

    return facet;
}

void *Depot::GetFacet(UID uid, FacetType type)
{
    if (!uid) return 0;

    //if (!bitmapByUid[uid].test(type)) {
    //    return 0;
    //}

#if 1
    hashesThisFrame++;
    if (!indexByUid[type].contains(uid)) {
        return 0;
    }

    size_t index = indexByUid[type][uid];
#else
    const auto &val = indexByUid[type].find(uid);
    if (val == indexByUid[type].end()) {
        return 0;
    }

    size_t index = val->second; // indexByUid[type][uid];
#endif
    switch (type) {
        case Facet_Attach:        return &attach        [index];
        case Facet_Body:          return &body          [index];
        case Facet_Card:          return &card          [index];
        case Facet_CardProto:     return &cardProto     [index];
        case Facet_Combat:        return &combat        [index];
        case Facet_Cursor:        return &cursor        [index];
        case Facet_Deck:          return &deck          [index];
        case Facet_EffectList:    return &effectList    [index];
        case Facet_Font:          return &font          [index];
        case Facet_FpsCounter:    return &fpsCounter    [index];
        case Facet_Keymap:        return &keymap        [index];
        case Facet_Material:      return &material      [index];
        case Facet_MaterialProto: return &materialProto [index];
        case Facet_Position:      return &position      [index];
        case Facet_Sound:         return &sound         [index];
        case Facet_Sprite:        return &sprite        [index];
        case Facet_Spritesheet:   return &spritesheet   [index];
        case Facet_Text:          return &text          [index];
        case Facet_Texture:       return &texture       [index];
        case Facet_TriggerList:   return &triggerList   [index];
        default: assert(!"what is that, mate?");
    }
    return 0;
}

void *Depot::GetFacetByName(const char *name, FacetType type)
{
    std::string key{ name };
    if (uidByName.contains(key)) {
        UID uid = uidByName[key];
        return GetFacet(uid, type);
    }
    return 0;
}

static void DebugFontAtlas(Depot &depot, DrawQueue &drawQueue)
{
    // Debug display for font atlases
    int atlasOffsetY = 10;

    for (Font &font : depot.font) {
        if (!font.glyphCache.atlasSurface)
            continue;

        rect rect{};
        rect.x = SCREEN_W - font.glyphCache.atlasSurface->w - 10;
        rect.y = atlasOffsetY;
        rect.w = font.glyphCache.atlasSurface->w;
        rect.h = font.glyphCache.atlasSurface->h;

        DrawCommand drawGlyphAtlasBg{};
        drawGlyphAtlasBg.uid = font.uid;
        drawGlyphAtlasBg.dstRect = rect;
        drawGlyphAtlasBg.color = C255(COLOR_WHEAT);
        drawGlyphAtlasBg.color.a = 0.8f * 255.0f;
        drawGlyphAtlasBg.depth = 0;
        drawQueue.push_back(drawGlyphAtlasBg);

        DrawCommand drawGlyphAtlas{};
        drawGlyphAtlas.uid = font.uid;
        drawGlyphAtlas.dstRect = rect;
        drawGlyphAtlas.texture = font.glyphCache.atlasTexture;
        drawGlyphAtlas.depth = 1;
        drawQueue.push_back(drawGlyphAtlas);

        atlasOffsetY += rect.h + 10;
    }
}

void Depot::Run(void)
{
    while (renderSystem.Running()) {
        frame++;

        // Time is money
        int physicsIters = 0;
        {
            nowPrev = now;
            now = clock_now();
            realDt = now - nowPrev;
            double cappedRealDt = MIN(realDt, fixedDt);  // cap to prevent spiral while debugging
            realDtSmooth = LERP(realDtSmooth, cappedRealDt, 0.1);

            physicsAccum += cappedRealDt;
            while (physicsAccum >= fixedDt) {
                physicsIters++;
                physicsAccum -= fixedDt;
            }
        }

        // Update game state
        BeginFrame();

        cursorSystem.UpdateCursors(*this);
        histogramSystem.Update(*this);

        // TODO: Think about pub-sub as a better form of message bus than this
        // MsgQueue thing that gets read/written everywhere.

        // TODO: If something has a side effect, defer it when possible by
        // adding it to its own dedicated queue as oppposed to mixing
        // everything into One Big Queue (TM). E.g. collisionList essentially
        // does this right now, but has a lot of O(n) iterations that we could
        // maybe remove.

        {
            // TODO: Stop using a queue for this, just capture state of every
            // key / mouse / quit request into a map (handle instantaneous
            // down/up events properly, they should report down for 1 frame).

            // Collect SDL events into the appropriate queues
            eventSystemSDL.ProcessEvents(inputQueue);

            // TODO(dlb): Top-down stack order (e.g. menu handles input before game
            // when open, and marks all inputs as handled to prevent it from leaking
            // into the game, and a UI window like inventory does the same, but
            // only if the mouse is within the window bounds or smth)
            //
            // Translate inputs into messages using the active keymap(s)
            inputSystem.ProcessInput(*this, inputQueue);
            inputQueue.clear();
        }

        movementSystem.React(*this);  // reacts to Movement_*

        for (int i = 0; i < physicsIters; i++) {
            physicsSystem.Update(*this, fixedDt);
        }

        collisionSystem.DetectCollisions(*this, collisionList);

        cursorSystem.UpdateDragTargets(*this, collisionList);  // gen: card_dragbegin, card_dragupdate, card_leftclick

        // Update cards based on their parent positions
        cardSystem.UpdateCards(*this);                         // gen: sprite_updateanim, card_dragend (hack for first bounce)
        cardSystem.UpdateStacks(*this, collisionList);
        effectSystem.ApplyDragFx(*this, collisionList);

        // Message converter
        triggerSystem.React(*this);   // reacts to *, generates *

        // Pure message reactors (do not modify msgQueue here!)
        cardSystem.React(*this);      // reacts to Card
        spriteSystem.React(*this);    // reacts to Sprite
        audioSystem.React(*this);     // reacts to Audio
        textSystem.React(*this);      // reacts to Text
        renderSystem.React(*this);    // reacts to Render

        spriteSystem.Update(*this);
        renderSystem.UpdateCachedTextures(*this);

        // Populate draw queue(s)
        DrawQueue cardQueue{};
        DrawQueue histogramQueue{};
        DrawQueue textQueue{};
        DrawQueue dbgAtlasQueue{};
        cardSystem.Display(*this, cardQueue);
        combatSystem.Display(*this, cardQueue);
        histogramSystem.Display(*this, histogramQueue);
        textSystem.Display(*this, textQueue);

#if FDOV_DEBUG_GLYPH_ATLAS
        DebugFontAtlas(*this, dbgAtlasQueue);
#endif
#if 0
        // Debug print for message queue
        if (inputQueue.size() || msgQueue.size()) {
            printf("Frame #%llu\n", frame);
            if (inputQueue.size()) {
                printf("  input   : ");
                for (const auto &input : inputQueue) {
                    printf(" %3d%s", input.scancode, input.down ? "v" : "^");
                }
                putchar('\n');
            }
            if (msgQueue.size()) {
                printf("  messages: ");
                for (const auto &msg : msgQueue) {
                    printf(" %3d", msg.type);
                }
                putchar('\n');
            }
            putchar('\n');
        }
#endif

        // Render draw queue(s)
        renderSystem.Clear(C_GRASS);
        renderSystem.Flush(*this, cardQueue);
        renderSystem.Flush(*this, histogramQueue);
        renderSystem.Flush(*this, textQueue);
        renderSystem.Flush(*this, dbgAtlasQueue);
        renderSystem.Present();
        EndFrame();

        // If you disable v-sync, you'll want this to prevent global warming
        //SDL_Delay(1);
    }
}