#include "bus.h"
#include "io.inl"

namespace Bus {

    void State::initialize() {
        // reset memory regions
        memset(iTcm, 0, sizeof(iTcm));
        memset(dTcm, 0, sizeof(dTcm));
        memset(mainRam, 0, sizeof(mainRam));
        memset(sharedRam, 0, sizeof(sharedRam));
        memset(arm7Ram, 0, sizeof(arm7Ram));
        memset(vram, 0, sizeof(vram));

        // reset io
        for (int i = 0x4000000; i <= 0x40005C8; i++)
            io9Write8(i, 0);
    }

}