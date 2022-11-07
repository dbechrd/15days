#include "depot.h"
#include "SDL/SDL.h"
#include <cstdio>

int main(int argc, char *argv[])
{
    for (int i = 0; i < argc; i++) {
        printf("argv[%d] = %s\n", i, argv[i]);
    }

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    SDL_Window *window = SDL_CreateWindow(
        "15days",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1600,
        900,
        SDL_WINDOW_OPENGL
    );

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

    int videoDrivers = SDL_GetNumVideoDrivers();
    for (int i = 0; i < videoDrivers; i++) {
        printf("video_driver[%d] = %s\n", i, SDL_GetVideoDriver(i));
    }

    int drivers = SDL_GetNumRenderDrivers();
    for (int i = 0; i < drivers; i++) {
        SDL_RendererInfo info{};
        if (SDL_GetRenderDriverInfo(i, &info) < 0) {
            printf("Failed to query render_driver[%d] info: %s\n", i, SDL_GetError());
            continue;
        }

        printf("render_driver[%d]:\n"
            "  name                : %s\n"
            "  num_texture_formats : %u\n"
            "  max_texture_width   : %d\n"
            "  max_texture_height  : %d\n",
            i,
            info.name,
            info.num_texture_formats,
            info.max_texture_width,
            info.max_texture_height
        );
    }
#endif

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    printf("Audio driver: %s\n", SDL_GetCurrentAudioDriver());
    printf("Video driver: %s\n", SDL_GetCurrentVideoDriver());

    // Useful to get window width/height before toggling fullscreen
    //SDL_GetCurrentDisplayMode();

    bool quit = false;
    SDL_Event evt{};
    while (!quit) {
        while (SDL_PollEvent(&evt)) {
            switch (evt.type) {
                case SDL_KEYDOWN: {
                    switch (evt.key.keysym.scancode) {
                        case SDL_SCANCODE_ESCAPE: {
                            quit = true;
                            break;
                        }
                        default: {
                            break;
                        }
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 15, 50, 70, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(renderer, 0);

        SDL_RenderPresent(renderer);
        SDL_Delay(10);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    printf("SDL reported %d unfreed allocations\n", SDL_GetNumAllocations());
    return 0;
}
