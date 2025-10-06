#include "frontend.h"
#include "../../core/bus/bus.h"
#include "../../core/bus/read_write.inl"

#define rgb15to24(color) ((((color) >> 0) & 0x1f) << 19) | ((((color) >> 5) & 0x1f) << 11) | ((((color) >> 10) & 0x1f) << 3)

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

    ds->updateJoypadWithKeyboard(keyboard);

    u32 msBefore = SDL_GetTicks();
    for (int i = 0; i < 558566; i++)
        ds->execute();
    u32 msFrameTook = SDL_GetTicks() - msBefore;

    char fpsTitle[64];
    snprintf(fpsTitle, sizeof fpsTitle, "[%.2f] lilds", 1000.0 / (float)(msFrameTook));
    SDL_SetWindowTitle(window, fpsTitle);

    // [DEBUG] lcdc draw
    if (ds->bus) {
        Bus::State* bus = static_cast<Bus::State*>(ds->bus);
        for (int y = 0; y < dsFramebufferH/2; y++) {
            for (int x = 0; x < dsFramebufferW; x++) {
                int ind = x + y * dsFramebufferW;
                u32 addr = 0x0680'0000 + 2 * ind;
                u16 data = bus->arm9Read<u16, Arm::AccessType::Data, true>(nullptr, addr, Arm::Access::N);
                u32 color = rgb15to24(data) | 0xff00'0000;
                dsFramebuffer[ind] = SDL_Swap32(color);
            }
        }
    }

    SDL_UpdateTexture(texture, NULL, dsFramebuffer, dsFramebufferBPP * dsFramebufferW);

    // Render
    SDL_RenderClear(windowRenderer);
    SDL_RenderCopy(windowRenderer, texture, NULL, NULL);
    SDL_RenderPresent(windowRenderer);

    return false;
}