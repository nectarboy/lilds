#include "bus.h"

namespace Bus {

    void State::initialize() {
        // reset memory regions
        memset(instTcm, 0, sizeof(instTcm));
        memset(dataTcm, 0, sizeof(dataTcm));
        memset(mainRam, 0, sizeof(mainRam));
        memset(sharedRam, 0, sizeof(sharedRam));
        memset(vram, 0, sizeof(vram));

        // reset io
        for (int i = 0x4000000; i <= 0x40005C8; i++)
            io9Write8(i, 0);
    }

}