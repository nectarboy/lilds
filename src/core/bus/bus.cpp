#include "bus.h"

namespace Bus {

    u8 State::arm9Read8(Arm::State*, u32 addr, Arm::Access access) {
        int region = addr >> 24;
        switch (region) {
            // Main Memory
            case 2: {
                return mainRam[addr & 0x3fffff];
                break;
            }
        }
    }

}