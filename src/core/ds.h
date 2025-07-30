#pragma once
#include "../helpers.h"
// #include "arm/arm.h"
// #include "bus/bus.h"

namespace DS {

    struct State {
        State() {
            createComponents();
        }

        // references
        void* arm7 = nullptr;
        void* arm9 = nullptr;
        void* bus = nullptr;

        // methods
        void createComponents();
    };

}