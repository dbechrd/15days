#include "depot.h"
#include <cassert>

UID Depot::Alloc(void)
{
    UID uid = nextUid;
    nextUid++;
    return uid;
}

void *Depot::AddFacet(UID uid, FacetType type)
{
    void *existingFacet = GetFacet(uid, type);
    if (existingFacet) {
        printf("WARN: AddFacet called more than once for same uid/type pair.\n");
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
        EMPLACE(Facet_Attach,       attach);
        EMPLACE(Facet_Body,         body);
        EMPLACE(Facet_Combat,       combat);
        EMPLACE(Facet_Keymap,       keymap);
        EMPLACE(Facet_Position,     position);
        EMPLACE(Facet_Sound,        sound);
        EMPLACE(Facet_Sprite,       sprite);
        EMPLACE(Facet_Text,         text);
        EMPLACE(Facet_Trigger,      trigger);
        EMPLACE(Facet_TriggerList,  triggerList);
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

    indexByUid[type][uid] = index;
    facet->uid = uid;
    facet->type = type;
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
        case Facet_Keymap:      return &keymap      [index];
        case Facet_Position:    return &position    [index];
        case Facet_Sound:       return &sound       [index];
        case Facet_Sprite:      return &sprite      [index];
        case Facet_Text:        return &text        [index];
        case Facet_Trigger:     return &trigger     [index];
        case Facet_TriggerList: return &triggerList [index];
        default: assert(!"what is that, mate?");
    }
    return 0;
}
