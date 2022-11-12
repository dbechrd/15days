#include "audio_system.h"
#include "SDL/SDL.h"
#include <cstdio>

FDOVResult AudioSystem::Init(void)
{
    int sdl_init_err = SDL_Init(SDL_INIT_AUDIO);
    if (sdl_init_err < 0) {
        printf("Failed to initialize SDL audio: %s\n", SDL_GetError());
        return FDOV_INIT_FAILED;
    }

#if 0
    int audioDrivers = SDL_GetNumAudioDrivers();
    for (int i = 0; i < audioDrivers; i++) {
        printf("audio_driver[%d] = %s\n", i, SDL_GetAudioDriver(i));
    }

    int audioDevices = SDL_GetNumAudioDevices(0);
    for (int i = 0; i < audioDevices; i++) {
        SDL_AudioSpec spec{};
        if (SDL_GetAudioDeviceSpec(i, 0, &spec) != 0) {
            printf("Failed to query audio_device[%d] spec: %s\n", i, SDL_GetError());
            continue;
        }

        printf("audio_device[%d]:\n"
            "  freq           : %d\n"
            "  channels       : %u\n"
            "  buffer samples : %d (per channel)\n"
            "  buffer size    : %u bytes\n",
            i,
            spec.freq,
            spec.channels,
            spec.samples,
            spec.size
        );
    }
#endif

    printf("Audio driver: %s\n", SDL_GetCurrentAudioDriver());
    return FDOV_SUCCESS;
}