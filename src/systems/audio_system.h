#pragma once
#include "../common/basic.h"
#include "../common/error.h"
#include "../facets/sound.h"

struct AudioSystem {
    Sound *FindOrLoadSound(Depot &depot, const char *soundKey);

    FDOVResult Init(void);
    void DestroyDepot(const Depot &depot);
    void Destroy(void);


    void React(Depot &depot);

private:
    //SDL_AudioDeviceID playbackDeviceId{};
    SoLoud::Soloud gSoloud; // SoLoud engine

    void PlaySound(Depot &depot, UID soundUid, bool override = false);
    void StopSound(Depot &depot, UID soundUid);
};