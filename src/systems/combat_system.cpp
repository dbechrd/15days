#include "combat_system.h"
#include "../common/message.h"
#include "../facets/depot.h"

void CombatSystem::React(double now, Depot &depot)
{
    size_t size = depot.msgQueue.size();
    for (int i = 0; i < size; i++) {
        Message &msg = depot.msgQueue[i];
        Combat *combat = (Combat *)depot.GetFacet(msg.uid, Facet_Combat);
        if (!combat) {
            continue;
        }

        bool canAttack = !(combat->attackStartedAt || combat->defendStartedAt);
        bool canDefend = !(combat->attackStartedAt);

        switch (msg.type) {
            case MsgType_Input_Primary:
            {
                if (canAttack) {
                    combat->attackStartedAt = now;
                    combat->attackCooldown = 0.2;
                }
                break;
            }
            case MsgType_Input_Secondary: {
                if (canDefend) {
                    combat->defendStartedAt = now;
                    combat->defendCooldown = 0.6;
                }
                break;
            }
            default: break;
        }
    }
}

void CombatSystem::Behave(double now, Depot &depot, double dt)
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