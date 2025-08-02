#include "bus.h"

namespace Bus {

    void State::initialize() {
        memset(instTcm, 0, sizeof(instTcm));
        memset(dataTcm, 0, sizeof(dataTcm));
        memset(mainRam, 0, sizeof(mainRam));
        memset(sharedRam, 0, sizeof(sharedRam));
        memset(vram, 0, sizeof(vram));
    }

}