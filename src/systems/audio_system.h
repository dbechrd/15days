#pragma once
#include "../common/basic.h"
#include "../common/error.h"
#include "../facets/sound.h"

struct AudioSystem {
    FDOVResult Init(void);
    void DestroyDepot(const Depot &depot);
    void Destroy(void);

    UID LoadSound(Depot &depot, const char *filename);

    void React(Depot &depot);

private:
    //SDL_AudioDeviceID playbackDeviceId{};
    SoLoud::Soloud gSoloud; // SoLoud engine

    void InitSound(Depot &depot, Sound &sound, const char *filename);
    void PlaySound(Depot &depot, UID soundUid, bool override = false);
    void StopSound(Depot &depot, UID soundUid);
};