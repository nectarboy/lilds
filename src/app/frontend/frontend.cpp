#include "frontend.h"

void Frontend::attachDsCore(DS::State* _ds) {
    ds = _ds;
    printf("Attached Core to Frontend\n");
}

bool Frontend::run() {
    // Wait / Check for events
    while (SDL_PollEvent(&sdlEvent)) {
        switch (sdlEvent.type) {
            // General
            case SDL_QUIT:
                return true;
                break;

            // Input
            case SDL_KEYDOWN:
                keyboard[sdlEvent.key.keysym.sym] = true;
                break;
            case SDL_KEYUP:
                keyboard[sdlEvent.key.keysym.sym] = false;
                break;
        }
    }

    u32 msBefore = SDL_GetTicks();
    for (int i = 0; i < 558566; i++)
        ds->execute();
    u32 msFrameTook = SDL_GetTicks() - msBefore;

    char fpsTitle[100];
    sprintf(fpsTitle, "[%.2f] lilds \n", 1000.0 / (float)(msFrameTook));
    SDL_SetWindowTitle(window, fpsTitle);

    // SDL_UpdateTexture(texture, NULL, frameBuffer, BUFFER_BYTES_PER_ROW);

    // Render
    // SDL_RenderClear(windowRenderer);
    // SDL_RenderCopy(windowRenderer, texture, NULL, NULL);
    // SDL_RenderPresent(windowRenderer);

    return false;
}