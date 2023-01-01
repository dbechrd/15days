#include "trigger_system.h"
#include "../facets/depot.h"

void TriggerSystem::React(Depot &depot)
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
    for (const Trigger &trigger : triggerList.triggers) {
        if (trigger.trigger == MsgType_Special_RelayAllMessages) {
            if (!msg.uid) {
                // Don't relay global messages that were already broadcasted
                continue;
            }

            if (trigger.callback) {
                trigger.callback(depot, msg, trigger, trigger.userData);
            } else {
                Message relayMsg = msg;
                relayMsg.uid = trigger.message.uid;
                depot.msgQueue.push_back(relayMsg);
            }
        } else if (trigger.trigger == msg.type) {
            if (trigger.callback) {
                trigger.callback(depot, msg, trigger, trigger.userData);
            } else {
                depot.msgQueue.push_back(trigger.message);
            }
        }
    }
}