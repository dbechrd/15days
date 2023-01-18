#include "audio_system.h"
#include "../facets/depot.h"

#include "../common/missing_ogg.h"

Sound *AudioSystem::FindOrLoadSound(Depot &depot, const char *soundKey)
{
    // Check if already loaded
    Sound *existingSound = (Sound *)depot.GetFacetByName(soundKey, Facet_Sound);
    if (existingSound) {
        return existingSound;
    }

    const ResourceDB::Sound *dbSound = depot.resources->sounds()->LookupByKey(soundKey);
    const char *path = dbSound->path()->c_str();

    SoLoud::Wav *wav = new SoLoud::Wav{};
    SoLoud::result err = wav->load(path);
    if (err) {
        SDL_LogError(0, "Failed to load wav: %s\n  %u\n", path, err);

        err = wav->loadMem(missing_ogg_bytes, sizeof(missing_ogg_bytes), false, false);
        if (err) {
            SDL_LogError(0, "Failed to load fallback sound:\n  result = %d\n", err);
            DLB_ASSERT(!"Missing sound also failed to load.. uh-oh no fallback!");
            return 0;
        }
    }

    UID uidSound = depot.Alloc(soundKey);
    Sound *sound = (Sound *)depot.AddFacet(uidSound, Facet_Sound);
    sound->soundKey = soundKey;
    sound->wav = wav;
    return sound;
}

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
    SoLoud::result err = gSoloud.init(); //gSoloud.FLAGS::CLIP_ROUNDOFF, gSoloud.BACKENDS::SDL2);
    if (err) {
        SDL_Log("SoLoud failed to init with error code %u\n", err);
        return FDOV_INIT_FAILED;
    }

    SDL_Log("SoLoud init with backend: %s", gSoloud.mBackendString);

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
    // Destructor will do this:
    gSoloud.stopAll();
    gSoloud.deinit();
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
        DLB_ASSERT(sound->wav->mSampleCount);
        gSoloud.play(*sound->wav, 1.0f);
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