#pragma once
#include "../common/basic.h"
#include "../common/draw_list.h"

struct Text_UpdateNarratorRequest {
    const char * str        {};
    vec2         offset     {};
    vec4         color      {};
    double       displayFor {};
};

typedef std::deque<Text_UpdateNarratorRequest> UpdateNarratorQueue;

struct TextSystem {
    struct Font *FindOrLoadFont(Depot &depot, const char *fontKey);

    void Init(Depot &depot);

    void PushUpdateText(Depot &depot, UID uidText, vec2 offset, vec4 color,
        const char *str, bool strOwner = false);
    void PushUpdateNarrator(Depot &depot, vec2 offset, vec4 color,
        const char *str, double displayFor = 3.0, bool interrupt = false);

    void Update(Depot &depot);

    void Display(Depot &depot, DrawQueue &drawQueue);

private:
    UpdateNarratorQueue updateNarratorQueue{};
    UID uidNarrator{};
    double narratorMsgStartedAt = 0;

    UID CreateNarrator(Depot &depot);
};