#pragma once
#include "../helpers.h"

namespace DS {

    struct State {
        State() {
            createComponents();
            initialize();
        }

        // references
        void* arm7 = nullptr;
        void* arm9 = nullptr;
        void* bus = nullptr;
        void* cp15 = nullptr;

        // initialization methods
        void createComponents();
        void attachFrontend(void* _frontend);
        void initialize();


        // loading methods
        void loadRomFileIntoMainMem(std::vector<char>& romFile);

        // execution methods
        void execute();
    };

}