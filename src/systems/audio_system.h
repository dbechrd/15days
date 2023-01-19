#pragma once
#include "../common/basic.h"
#include "../common/message.h"
#include "../facets/sound.h"

struct Audio_PlaySoundRequest {
    const char * soundKey {};
    bool         override {};
};
struct Audio_StopSoundRequest {
    const char * soundKey {};
};

typedef std::vector<Audio_PlaySoundRequest> Audio_PlaySoundQueue;
typedef std::vector<Audio_StopSoundRequest> Audio_StopSoundQueue;

struct AudioSystem {
    Sound *FindOrLoadSound(Depot &depot, const char *soundKey);

    Error Init(void);
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

    void PlaySoundInternal(Depot &depot, const Audio_PlaySoundRequest &playSoundRequest);
    void StopSoundInternal(Depot &depot, const Audio_StopSoundRequest &stopSoundRequest);
};