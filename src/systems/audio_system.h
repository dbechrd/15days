#pragma once
#include "../common/basic.h"
#include "../common/error.h"
#include "../facets/sound.h"

struct AudioSystem {
    FDOVResult Init(void);
    void DestroyDepot(const Depot &depot);
    void Destroy(void);
    void React(double now, Depot &depot);
    void Behave(double now, Depot &depot, double dt);
    //void Display(double now, Depot &depot, DrawQueue &drawQueue);

    static void InitSound(Sound &sound, std::string &filename);
private:
    SDL_AudioDeviceID playbackDeviceId{};

    void PlaySound(Depot &depot, UID soundUid, bool override = false);
};