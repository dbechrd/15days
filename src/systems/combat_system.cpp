#include "combat_system.h"
#include "../facets/depot.h"
#include "../facets/sprite.h"
#include <cassert>

void CombatSystem::ProcessCommands(double now, Depot &depot, UID uid, const CommandQueue &commandQueue)
{
    Combat *combat = (Combat *)depot.GetFacet(uid, Facet_Combat);
    if (!combat) return;

    for (const CommandType &command : commandQueue) {
        switch (command) {
            case Command_Primary:
            {
                combat->attackStartedAt = now;
                combat->attackDuration = 0.2;
                break;
            }
            default: break;
        }
    }
}

void CombatSystem::Update(double now, Depot &depot)
{
    for (Combat &combat : depot.combat) {
        if (combat.attackStartedAt) {
            assert(combat.attackDuration);
            float attackAlpha = (now - combat.attackStartedAt) / combat.attackDuration;

            if (attackAlpha < 1.0) {
                // TOOD: Something interesting during attack
            } else {
                combat.attackStartedAt = 0;
                combat.attackDuration = 0;
            }
        }
    }
}