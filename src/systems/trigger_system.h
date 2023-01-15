#pragma once
#include "../common/basic.h"
#include "../common/message.h"
#include "../facets/trigger.h"

struct TriggerSystem {
    void Trigger_Audio_PlaySound(Depot &depot, UID subject, MsgType msgType, const char *soundFile, bool override = true);
    void Trigger_Audio_StopSound(Depot &depot, UID subject, MsgType msgType, const char *soundFile);
    void Trigger_Render_Screenshake(Depot &depot, UID subject, MsgType msgType, float amount, float freq, double duration);
    void Trigger_Text_UpdateText(Depot &depot, UID src, MsgType msgType, UID dst, const char *str, vec4 color);
    void Trigger_Sprite_UpdateAnimation(Depot &depot, UID src, MsgType msgType, UID dst, const char *anim_name, TriggerCallback callback = 0, void *userData = 0);
    void Trigger_Special_RelayAllMessages(Depot &depot, UID src, UID dst);

    void React(Depot &depot);

private:
    void CheckTriggers(Depot &depot, TriggerList &triggerList, Message &msg);
};