#pragma once
#include "../../helpers.h"
#include "SDL.h"

struct Frontend {

    // framebuffer
    static const int dsFramebufferW = 256;
    static const int dsFramebufferH = 384;
    static const int dsFramebufferBPP = 4;
    u8 dsFramebuffer[dsFramebufferW * dsFramebufferH * dsFramebufferBPP];

    // SDL structs
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

    // destructor
    ~Frontend() {
        SDL_DestroyRenderer(windowRenderer);
        SDL_DestroyWindow(window);
        SDL_DestroyTexture(texture);
    }
};