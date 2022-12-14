#include "audio_system.h"
#include "../facets/depot.h"

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
#if 0
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
    if (!playbackDeviceId) {
        SDL_Log("Failed to open audio device: %s\n", SDL_GetError());
        return FDOV_INIT_FAILED;
    }

    printf("\nobtained playback device:\n");
    PrintSDLAudioSpec(obtainedPlaybackSpec);

    SDL_PauseAudioDevice(playbackDeviceId, 0);

    printf("\nAudio driver: %s\n", SDL_GetCurrentAudioDriver());
    return FDOV_SUCCESS;
#else
    SoLoud::result res = gSoloud.init();
    if (res) {
        SDL_Log("SoLoud failed to init with error code %u\n", res);
        return FDOV_INIT_FAILED;
    }

    gSoloud.setGlobalVolume(0.2f);

    return FDOV_SUCCESS;
#endif
}

void AudioSystem::DestroyDepot(const Depot &depot)
{
    //for (const Sound &sound : depot.sound) {
    //    SDL_FreeWAV(sound.data);
    //}
}

void AudioSystem::Destroy(void)
{
#if 0
    SDL_CloseAudioDevice(playbackDeviceId);
#else
    gSoloud.deinit();
#endif
}

UID AudioSystem::LoadSound(Depot &depot, const char *filename)
{
    // Check if already loaded
    Sound *existingSound = (Sound *)depot.GetFacetByName(filename, Facet_Sound);
    if (existingSound) {
        return existingSound->uid;
    }

    // Load a new audio buffer
    UID uidSound = depot.Alloc(filename);
    Sound *sound = (Sound *)depot.AddFacet(uidSound, Facet_Sound);
    InitSound(depot, *sound, filename);
    return uidSound;
}

void AudioSystem::InitSound(Depot &depot, Sound &sound, const char *filename)
{
#if 0
    SDL_LoadWAV(filename.c_str(), &sound.spec, &sound.data, &sound.data_length);
    if (!sound.data) {
        printf("Failed to load wav: %s\n  %s\n", filename.c_str(), SDL_GetError());
    }
#else
    sound.filename = filename;
    sound.wav = (SoLoud::Wav *)depot.resourceArena.Alloc(sizeof(SoLoud::Wav));
    new (sound.wav) SoLoud::Wav;
    SoLoud::result res = sound.wav->load(filename);
    if (res) {
        printf("Failed to load wav: %s\n  %u\n", filename, res);
    }
#endif
}

void AudioSystem::PlaySound(Depot &depot, UID soundUid, bool override)
{
    Sound *sound = (Sound *)depot.GetFacet(soundUid, Facet_Sound);
    if (!sound) {
        printf("WARN: Sound missing for uid: %u\n", soundUid);
        return;
    }

    // Check if this sound is already playing
    if (gSoloud.countAudioSource(*sound->wav)) {
        // If user wants to override it
        if (override) {
            // restart sound effect
            gSoloud.stopAudioSource(*sound->wav);
            gSoloud.play(*sound->wav);
        }
        // implicit else: don't play it again, wait until it finishes
    } else {
        // play sound effect
        gSoloud.play(*sound->wav);
    }
}

void AudioSystem::StopSound(Depot &depot, UID soundUid)
{
    Sound *sound = (Sound *)depot.GetFacet(soundUid, Facet_Sound);
    if (!sound) {
        printf("WARN: Sound missing for uid: %u\n", soundUid);
        return;
    }

    gSoloud.stopAudioSource(*sound->wav);
}

void AudioSystem::React(Depot &depot)
{
    size_t size = depot.msgQueue.size();
    for (int i = 0; i < size; i++) {
        Message msg = depot.msgQueue[i];
        //Sound *sound = (Sound *)depot.GetFacet(msg.uid, Facet_Sound);
        //if (!sound) {
        //    continue;
        //}

        switch (msg.type) {
            case MsgType_Audio_PlaySound:
            {
                PlaySound(depot, msg.uid, msg.data.audio_playsound.override);
                break;
            }
            case MsgType_Audio_StopSound:
            {
                StopSound(depot, msg.uid);
                break;
            }
            default: break;
        }
    }
}