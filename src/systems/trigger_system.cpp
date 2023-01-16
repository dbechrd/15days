#include "trigger_system.h"
#include "../facets/depot.h"


void TriggerSystem::Trigger_Audio_PlaySound(Depot &depot, UID subject, MsgType msgType,
    const char *soundFile, bool override)
{
    TriggerList *triggerList = (TriggerList *)depot.AddFacet(subject, Facet_TriggerList, false);

    Trigger trigger{};
    trigger.trigger = msgType;
    trigger.message.uid = depot.audioSystem.LoadSound(depot, soundFile);
    trigger.message.type = MsgType_Audio_PlaySound;
    trigger.message.data.audio_playsound.override = override;
    triggerList->triggers.push_back(trigger);
}

void TriggerSystem::Trigger_Audio_StopSound(Depot &depot, UID subject, MsgType msgType,
    const char *soundFile)
{
    TriggerList *triggerList = (TriggerList *)depot.AddFacet(subject, Facet_TriggerList, false);

    Trigger trigger{};
    trigger.trigger = msgType;
    trigger.message.uid = depot.audioSystem.LoadSound(depot, soundFile);
    trigger.message.type = MsgType_Audio_StopSound;
    triggerList->triggers.push_back(trigger);
}

void TriggerSystem::Trigger_Render_Screenshake(Depot &depot, UID subject, MsgType msgType,
    float amount, float freq, double duration)
{
    TriggerList *triggerList = (TriggerList *)depot.AddFacet(subject, Facet_TriggerList, false);

    Trigger trigger{};
    trigger.trigger = msgType;
    //trigger.message.uid = target_camera_uid;
    trigger.message.type = MsgType_Render_Screenshake;
    trigger.message.data.render_screenshake.amount = amount;
    trigger.message.data.render_screenshake.freq = freq;
    trigger.message.data.render_screenshake.duration = duration;
    triggerList->triggers.push_back(trigger);
}

void TriggerSystem::Trigger_Text_UpdateText(Depot &depot, UID src, MsgType msgType, UID dst,
    const char *str, vec4 color)
{
    TriggerList *triggerList = (TriggerList *)depot.AddFacet(src, Facet_TriggerList, false);

    Trigger trigger{};
    trigger.trigger = msgType;
    trigger.message.uid = dst;
    trigger.message.type = MsgType_Text_UpdateText;
    trigger.message.data.text_updatetext.str = str;
    trigger.message.data.text_updatetext.color = color;
    triggerList->triggers.push_back(trigger);
}

void TriggerSystem::Trigger_Sprite_UpdateAnimation(Depot &depot, UID src, MsgType msgType, UID dst,
    const char *animationKey, TriggerCallback callback, void *userData)
{
    TriggerList *triggerList = (TriggerList *)depot.AddFacet(src, Facet_TriggerList, false);

    Trigger trigger{};
    trigger.trigger = msgType;
    trigger.message.uid = dst;
    trigger.message.type = MsgType_Sprite_UpdateAnimation;
    trigger.message.data.sprite_updateanimation.animKey = animationKey;
    trigger.callback = callback;
    trigger.userData = userData;
    triggerList->triggers.push_back(trigger);
}

void TriggerSystem::Trigger_Special_RelayAllMessages(Depot &depot, UID src, UID dst)
{
    TriggerList *triggerList = (TriggerList *)depot.AddFacet(src, Facet_TriggerList, false);

    Trigger relayTrigger{};
    relayTrigger.trigger = MsgType_Special_RelayAllMessages;
    relayTrigger.message.uid = dst;
    triggerList->triggers.push_back(relayTrigger);
}


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