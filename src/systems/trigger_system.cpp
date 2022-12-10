#include "trigger_system.h"
#include "../facets/depot.h"

void TriggerSystem::React(double now, Depot &depot)
{
    for (int i = 0; i < depot.msgQueue.size(); i++) {
        Message msg = depot.msgQueue[i];

        if (msg.uid) {
            TriggerList *triggerList = (TriggerList *)depot.GetFacet(msg.uid, Facet_TriggerList);
            if (triggerList) {
                CheckTriggers(depot, *triggerList, msg);
            }
        } else {
            // Any trigger can be bound to a system message, so we broadcast
            // it to all trigger lists
            for (TriggerList &triggerList : depot.triggerList) {
                CheckTriggers(depot, triggerList, msg);
            }
        }
    }
}

void TriggerSystem::CheckTriggers(Depot &depot, TriggerList &triggerList, Message &msg)
{
    for (const UID &triggerUid : triggerList.triggers) {
        Trigger *trigger = (Trigger *)depot.GetFacet(triggerUid, Facet_Trigger);
        if (trigger) {
            if (trigger->trigger == msg.type) {
                if (trigger->callback) {
                    trigger->callback(depot, msg, *trigger, trigger->userData);
                } else {
                    depot.msgQueue.push_back(trigger->message);
                }
            }
        } else {
            printf("WARN: Trigger list for entity %u contained invalid trigger uid %u\n", msg.uid, triggerUid);
        }
    }
}