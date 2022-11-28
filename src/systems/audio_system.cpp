#include "audio_system.h"
#include "SDL/SDL.h"
#include <cstdio>

void PrintSDLAudioSpec(const SDL_AudioSpec &spec)
{
    char formatBuf[16]{};
    snprintf(CSTR(formatBuf), "%s %s %s %u",
        SDL_AUDIO_ISSIGNED(spec.freq) ? "S" : "U",
        SDL_AUDIO_ISBIGENDIAN(spec.freq) ? "BE" : "LE",
        SDL_AUDIO_ISFLOAT(spec.freq) ? "FLT" : "INT",
        SDL_AUDIO_BITSIZE(spec.freq)
    );

    printf(
        "  freq           : %d\n"
        "  format         : %s\n"
        "  channels       : %u\n"
        "  silence        : %u\n"
        "  buffer samples : %d (per channel)\n"
        "  buffer size    : %u bytes\n",
        spec.freq,
        formatBuf,
        spec.channels,
        spec.silence,
        spec.samples,
        spec.size
    );
}

FDOVResult AudioSystem::Init(void)
{
    int sdl_init_err = SDL_Init(SDL_INIT_AUDIO);
    if (sdl_init_err < 0) {
        printf("Failed to initialize SDL audio: %s\n", SDL_GetError());
        return FDOV_INIT_FAILED;
    }

#if 1
    int audioDrivers = SDL_GetNumAudioDrivers();
    for (int i = 0; i < audioDrivers; i++) {
        printf("audio_driver[%d] = %s\n", i, SDL_GetAudioDriver(i));
    }

    int playbackDeviceCount = SDL_GetNumAudioDevices(0);
    for (int i = 0; i < playbackDeviceCount; i++) {
        SDL_AudioSpec spec{};
        if (SDL_GetAudioDeviceSpec(i, 0, &spec) != 0) {
            printf("Failed to query playback_devices[%d] spec: %s\n", i, SDL_GetError());
            continue;
        }

        printf("\nplayback_devices[%d]:\n"
            "  name           : %s\n",
            i,
            SDL_GetAudioDeviceName(i, 0)
        );
        PrintSDLAudioSpec(spec);
    }

    int recordingDeviceCount = SDL_GetNumAudioDevices(1);
    for (int i = 0; i < recordingDeviceCount; i++) {
        SDL_AudioSpec spec{};
        if (SDL_GetAudioDeviceSpec(i, 0, &spec) != 0) {
            printf("Failed to query recording_devices[%d] spec: %s\n", i, SDL_GetError());
            continue;
        }

        char formatBuf[16]{};
        snprintf(CSTR(formatBuf), "%s %s %s %u",
            SDL_AUDIO_ISSIGNED(spec.freq) ? "S" : "U",
            SDL_AUDIO_ISBIGENDIAN(spec.freq) ? "BE" : "LE",
            SDL_AUDIO_ISFLOAT(spec.freq) ? "FLT" : "INT",
            SDL_AUDIO_BITSIZE(spec.freq)
        );

        printf("\nrecording_devices[%d]:\n"
            "  name           : %s\n",
            i,
            SDL_GetAudioDeviceName(i, 1)
        );
        PrintSDLAudioSpec(spec);
    }
#endif

    SDL_AudioSpec desiredPlaybackSpec{};
    desiredPlaybackSpec.freq = 48000;
    SDL_AudioSpec obtainedPlaybackSpec{};
    playbackDeviceId =
        SDL_OpenAudioDevice(0, 0, &desiredPlaybackSpec, &obtainedPlaybackSpec, 0);

    printf("\nobtained playback device:\n");
    PrintSDLAudioSpec(obtainedPlaybackSpec);

    SDL_PauseAudioDevice(playbackDeviceId, 0);

    printf("\nAudio driver: %s\n", SDL_GetCurrentAudioDriver());
    return FDOV_SUCCESS;
}

void AudioSystem::DestroyDepot(const Depot &depot)
{
    for (const Sound &sound : depot.sound) {
        SDL_FreeWAV(sound.audio_buf);
    }
}

void AudioSystem::Destroy(void)
{
    SDL_CloseAudioDevice(playbackDeviceId);
}

void AudioSystem::InitSound(Sound &sound, const char *filename)
{
    auto rwops = SDL_RWFromFile(filename, "rb");
    SDL_LoadWAV_RW(rwops, 0, &sound.spec, &sound.audio_buf, &sound.audio_len);
    SDL_RWclose(rwops);
    if (!sound.audio_buf) {
        printf("Failed to load audio file: %s\n  %s\n", filename, SDL_GetError());
    }
}

void AudioSystem::React(double now, Depot &depot, MsgQueue &msgQueue)
{
    size_t size = msgQueue.size();
    for (int i = 0; i < size; i++) {
        Message &msg = msgQueue[i];
        //Sound *sound = (Sound *)depot.GetFacet(msg.uid, Facet_Sound);
        //if (!sound) {
        //    continue;
        //}

        switch (msg.type) {
            case MsgType_Trigger_Sound_Play:
            {
                PlaySound(depot, msg.uid);
                break;
            }
            default: break;
        }
    }
}

void AudioSystem::Behave(double now, Depot &depot, double dt)
{

}

void AudioSystem::PlaySound(Depot &depot, UID soundUid)
{
    SDL_AudioStatus status = SDL_GetAudioDeviceStatus(playbackDeviceId);
    if (status != SDL_AUDIO_PLAYING) {
        return;
    }

    Sound *sound = (Sound *)depot.GetFacet(soundUid, Facet_Sound);
    if (!sound) {
        printf("WARN: Sound missing for uid: %u\n", soundUid);
        return;
    }

    SDL_ClearQueuedAudio(playbackDeviceId);
    SDL_QueueAudio(playbackDeviceId, sound->audio_buf, sound->audio_len);
}