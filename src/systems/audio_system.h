#pragma once
#include "../common/basic.h"
#include "../common/error.h"
#include "../common/message.h"
#include "../facets/sound.h"

typedef std::vector<Msg_Audio_PlaySoundRequest> Audio_PlaySoundQueue;
typedef std::vector<Msg_Audio_StopSoundRequest> Audio_StopSoundQueue;

struct AudioSystem {
    Sound *FindOrLoadSound(Depot &depot, const char *soundKey);

    FDOVResult Init(void);
    void DestroyDepot(const Depot &depot);
    void Destroy(void);

    void PushPlaySound(Depot &depot, const char *soundKey, bool override = false);
    void PushStopSound(Depot &depot, const char *soundKey);
    void ProcessQueues(Depot &depot);

private:
    //SDL_AudioDeviceID playbackDeviceId{};
    SoLoud::Soloud gSoloud; // SoLoud engine
    Audio_PlaySoundQueue playSoundQueue{};
    Audio_StopSoundQueue stopSoundQueue{};

    void PlaySoundInternal(Depot &depot, const Msg_Audio_PlaySoundRequest &playSoundRequest);
    void StopSoundInternal(Depot &depot, const Msg_Audio_StopSoundRequest &stopSoundRequest);
};