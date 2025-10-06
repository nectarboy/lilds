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
        void* joypad = nullptr;
        void* cp15 = nullptr;

        // initialization methods
        void createComponents();
        void attachFrontendComponents(std::map<int, bool>& frontendKeyboard);
        void initialize();

        // frontend methods
        void updateJoypadWithKeyboard(std::map<int, bool>& keyboard);

        // loading methods
        void loadRomFileIntoMainMem(std::vector<char>& romFile);

        // execution methods
        void execute();
    };

}