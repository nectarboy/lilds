#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <iostream>
#include <fstream>
#include <map>
#include <array>
#include <vector>
#include <bitset>
#include <algorithm>
#include <cassert>

//#include "../helpers.h"
#include "frontend/frontend.h"
#include "../core/ds.h"
#include "../core/arm/arm.h"
#include "../core/bus/bus.h"

int main(int argc, char* argv[]) {
    std::cout << __DATE__ << "\t" << __TIME__ << std::endl;

    DS::State lilds;
    std::cout << "Hello!" << std::endl;

    if (argc < 2) {
        std::cout << "Please input a rom." << std::endl;
        return 0;
    }

    // Load rom
    std::cout << "Rom path: " << argv[1] << std::endl;
    std::vector<char> romFile = getFileBinaryVector((std::string)argv[1]);
    lilds.loadRomFileIntoMainMem(romFile);

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL Error\n");
        return 1;
    }

    // Create frontend
    Frontend frontend;
    frontend.attachDsCore(&lilds);

    printf("Start execution\n\n");
    while (true) {
        bool quit = frontend.run();
        if (quit)
            break;
    }

    // Quit
    SDL_Quit();
    return 0;
}