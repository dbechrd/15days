#pragma once
#include "../common/basic.h"
#include "../common/error.h"
#include "../facets/sound.h"

struct AudioSystem {
    FDOVResult Init(void);
    void DestroyDepot(const Depot &depot);
    void Destroy(void);
    void React(double now, Depot &depot);
    //void Display(double now, Depot &depot, DrawQueue &drawQueue);

    static void InitSound(Depot &depot, Sound &sound, const char *filename);
private:
    //SDL_AudioDeviceID playbackDeviceId{};
    SoLoud::Soloud gSoloud; // SoLoud engine
    //SoLoud::Wav gWave;      // One wave file

    void PlaySound(Depot &depot, UID soundUid, bool override = false);
};