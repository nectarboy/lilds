#include "bus.h"

namespace Bus {

    void State::initialize() {
        memset(mainRam, 0, sizeof(mainRam));
        memset(sharedRam, 0, sizeof(sharedRam));
        memset(vram, 0, sizeof(vram));
    }

}