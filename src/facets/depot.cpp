#include "depot.h"
#include <cassert>

UID Depot::Alloc(void) {
    UID uid = nextUid;
    nextUid++;
    return uid;
}

void *Depot::AddFacet(UID uid, FacetType type) {
    size_t index = 0;
    Facet *facet = 0;
    switch (type) {
        case Facet_Attach:
        {
            index = attach.size();
            facet = &attach.emplace_back();
            break;
        }
        case Facet_Body:
        {
            index = body.size();
            facet = &body.emplace_back();
            break;
        }
        case Facet_Combat:
        {
            index = combat.size();
            facet = &combat.emplace_back();
            break;
        }
        case Facet_Keymap:
        {
            index = keymap.size();
            facet = &keymap.emplace_back();
            break;
        }
        case Facet_Position:
        {
            index = position.size();
            facet = &position.emplace_back();
            break;
        }
        case Facet_Sprite:
        {
            index = sprite.size();
            facet = &sprite.emplace_back();
            break;
        }
        case Facet_Trigger:
        {
            index = trigger.size();
            facet = &trigger.emplace_back();
            break;
        }
        default:
        {
            assert(!"what is that, mate?");
        }
    }

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

void *Depot::GetFacet(UID uid, FacetType type) {
    if (!indexByUid[type].contains(uid)) {
        return 0;
    }

    size_t index = indexByUid[type][uid];
    switch (type) {
        case Facet_Attach:   return &attach[index];
        case Facet_Body:     return &body[index];
        case Facet_Combat:   return &combat[index];
        case Facet_Keymap:   return &keymap[index];
        case Facet_Position: return &position[index];
        case Facet_Sprite:   return &sprite[index];
        case Facet_Trigger:  return &trigger[index];
        default: assert(!"what is that, mate?");
    }
    return 0;
}