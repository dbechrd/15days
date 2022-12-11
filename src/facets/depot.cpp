#include "depot.h"
#include "../common/clock.h"

UID Depot::Alloc(void)
{
    UID uid = nextUid;
    nextUid++;
    return uid;
}

void *Depot::AddFacet(UID uid, FacetType type, const char *name, bool warnDupe)
{
    void *existingFacet = GetFacet(uid, type);
    if (existingFacet) {
        if (warnDupe) {
            printf("WARN: AddFacet called more than once for same uid/type pair: %u/%d.\n", uid, type);
        }
        return existingFacet;
    }

    if (name) {
        std::string key{ name };
        if (indexByName[type].contains(key)) {
            printf("ERROR: There is already a facet of type %d with name %s. It has uid %u.\n",
                type, name, indexByName[type][key]);
            return 0;
        }
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
        EMPLACE(Facet_Attach,      attach);
        EMPLACE(Facet_Body,        body);
        EMPLACE(Facet_Combat,      combat);
        EMPLACE(Facet_Cursor,      cursor);
        EMPLACE(Facet_Deck,        deck);
        EMPLACE(Facet_Font,        font);
        EMPLACE(Facet_FpsCounter,  fpsCounter);
        EMPLACE(Facet_Keymap,      keymap);
        EMPLACE(Facet_Position,    position);
        EMPLACE(Facet_Sound,       sound);
        EMPLACE(Facet_Sprite,      sprite);
        EMPLACE(Facet_Spritesheet, spritesheet);
        EMPLACE(Facet_Text,        text);
        EMPLACE(Facet_Texture,     texture);
        EMPLACE(Facet_Trigger,     trigger);
        EMPLACE(Facet_TriggerList, triggerList);
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
    facet->type = type;
    if (name) {
        std::string key{ name };

        size_t keyLen = key.length();
        char *keyStr = (char *)resourceArena.Alloc(keyLen);
        snprintf(keyStr, keyLen, "%s", key.c_str());
        facet->name = keyStr;

        indexByName[type][key] = uid;
    }
    indexByUid[type][uid] = index;
    return facet;
}

void *Depot::GetFacet(UID uid, FacetType type)
{
    if (!indexByUid[type].contains(uid)) {
        return 0;
    }

    size_t index = indexByUid[type][uid];
    switch (type) {
        case Facet_Attach:      return &attach      [index];
        case Facet_Body:        return &body        [index];
        case Facet_Combat:      return &combat      [index];
        case Facet_Cursor:      return &cursor      [index];
        case Facet_Deck:        return &deck        [index];
        case Facet_Font:        return &font        [index];
        case Facet_FpsCounter:  return &fpsCounter  [index];
        case Facet_Keymap:      return &keymap      [index];
        case Facet_Position:    return &position    [index];
        case Facet_Sound:       return &sound       [index];
        case Facet_Sprite:      return &sprite      [index];
        case Facet_Spritesheet: return &spritesheet [index];
        case Facet_Text:        return &text        [index];
        case Facet_Texture:     return &texture     [index];
        case Facet_Trigger:     return &trigger     [index];
        case Facet_TriggerList: return &triggerList [index];
        default: assert(!"what is that, mate?");
    }
    return 0;
}

void *Depot::GetFacetByName(FacetType type, const char *name)
{
    std::string key{ name };
    if (indexByName[type].contains(key)) {
        UID uid = indexByName[type][key];
        return GetFacet(uid, type);
    }
    return 0;
}

void Depot::Run(void)
{
    while (renderSystem.Running()) {
        frame++;

        // Time is money
        nowPrev = now;
        now = clock_now();
        double realDt = now - nowPrev;
        realDt = MIN(realDt, fixedDt);  // cap to prevent spiral while debugging
        realDtSmooth = LERP(realDtSmooth, realDt, 0.1);

        physicsAccum += realDt;
        int physicsIters = 0;
        while (physicsAccum >= fixedDt) {
            physicsIters++;
            physicsAccum -= fixedDt;
        }

        // Update game state
        BeginFrame();

        cursorSystem.Update(*this);

        {
            // Collect SDL events into the appropriate queues
            eventSystemSDL.ProcessEvents(inputQueue, msgQueue);

            // TODO(dlb): Top-down stack order (e.g. menu handles input before game
            // when open, and marks all inputs as handled to prevent it from leaking
            // into the game, and a UI window like inventory does the same, but
            // only if the mouse is within the window bounds or smth)
            //
            // Translate inputs into messages using the active keymap(s)
            inputSystem.ProcessInput(now, *this, inputQueue);
            inputQueue.clear();
        }

        // TODO: Make these pure generators
        movementSystem.React(now, *this);  // reacts to Movement

        for (int i = 0; i < physicsIters; i++) {
            physicsSystem.Update(now, *this, fixedDt);
        }

        // Message converter
        triggerSystem.React(now, *this);   // reacts to *        generates *

        // Pure message reactors (do not modify msgQueue here!)
        audioSystem.React(now, *this);     // reacts to Audio
        spriteSystem.Update(now, *this);   // reacts to Sprite
        textSystem.React(now, *this);      // reacts to Text
        renderSystem.React(now, *this);    // reacts to Render

        renderSystem.UpdateCachedTextures(*this);

        // Populate draw queue(s)
        DrawQueue spriteQueue{};
        DrawQueue textQueue{};
        spriteSystem.Display(now, *this, spriteQueue);
        combatSystem.Display(now, *this, spriteQueue);
        textSystem.Display(now, *this, textQueue);

#if 0
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
        renderSystem.Flush(*this, spriteQueue);
        renderSystem.Flush(*this, textQueue);
        renderSystem.Present();
        EndFrame();

        // If you disable v-sync, you'll want this to prevent global warming
        //SDL_Delay(1);
    }
}