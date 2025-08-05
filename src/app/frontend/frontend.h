#pragma once
#include "../../helpers.h"
#include "../../core/ds.h"
#include "SDL.h"

struct Frontend {

    // ds reference
    DS::State* ds = nullptr;
    void attachDsCore(DS::State* _ds);

    // keyboard
    std::map<int, bool> keyboard;

    // framebuffer
    static const int dsFramebufferW = 256;
    static const int dsFramebufferH = 384;
    static const int dsFramebufferBPP = 4;
    u32 dsFramebuffer[dsFramebufferW * dsFramebufferH];
    

    // SDL
    SDL_Window* window = SDL_CreateWindow
    (
        "lilds",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        dsFramebufferW, dsFramebufferH,
        SDL_WINDOW_SHOWN
    );
    SDL_Renderer* windowRenderer = SDL_CreateRenderer
    (
        window,
        0,
        SDL_RENDERER_ACCELERATED
    );
    SDL_Texture* texture = SDL_CreateTexture
    (
        windowRenderer,
        SDL_PIXELFORMAT_ARGB32,
        SDL_TEXTUREACCESS_STREAMING,
        dsFramebufferW, dsFramebufferH
    );
    SDL_Event sdlEvent;

    // running methods
    bool run();

    // destructor
    ~Frontend() {
        SDL_DestroyRenderer(windowRenderer);
        SDL_DestroyWindow(window);
        SDL_DestroyTexture(texture);
    }
};