#include "trigger_system.h"

void TriggerSystem::React(double now, Depot &depot, MsgQueue &msgQueue)
{
    size_t size = msgQueue.size();
    for (int i = 0; i < size; i++) {
        Message &msg = msgQueue[i];
        TriggerList *triggerList = (TriggerList *)depot.GetFacet(msg.uid, Facet_TriggerList);
        if (!triggerList) {
            continue;
        }

        for (const UID &triggerUid : triggerList->triggers) {
            Trigger *trigger = (Trigger *)depot.GetFacet(triggerUid, Facet_Trigger);
            if (!trigger) {
                printf("WARN: Trigger list for entity %u contained invalid trigger uid %u\n", msg.uid, triggerUid);
                continue;
            }
            if (trigger->trigger == msg.type) {
                msgQueue.push_back(trigger->message);
            }
        }
    }
}