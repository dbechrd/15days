#include "combat_system.h"
#include "../facets/depot.h"
#include "../facets/sprite.h"
#include <cassert>

void CombatSystem::ProcessCommands(double now, Depot &depot, UID uid, const CommandQueue &commandQueue)
{
    Combat *combat = (Combat *)depot.GetFacet(uid, Facet_Combat);
    if (!combat) return;

    bool canAttack = !(combat->attackStartedAt || combat->defendStartedAt);
    bool canDefend = !(combat->attackStartedAt);

    for (const CommandType &command : commandQueue) {
        switch (command) {
            case Command_Primary:
            {
                if (canAttack) {
                    combat->attackStartedAt = now;
                    combat->attackCooldown = 0.2;
                }
                break;
            }
            case Command_Secondary: {
                if (canDefend) {
                    combat->defendStartedAt = now;
                    combat->defendCooldown = 1.0;
                }
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
            float attackAlpha = (now - combat.attackStartedAt) / combat.attackCooldown;
            if (attackAlpha < 1.0) {
                // TOOD: Something interesting during attack
            } else {
                combat.attackStartedAt = 0;
                combat.attackCooldown = 0;
            }
        }
        if (combat.defendStartedAt) {
            float defendAlpha = (now - combat.defendStartedAt) / combat.defendCooldown;
            if (defendAlpha < 1.0) {
                // TOOD: Something interesting during defend
            } else {
                combat.defendStartedAt = 0;
                combat.defendCooldown = 0;
            }
        }
    }
}