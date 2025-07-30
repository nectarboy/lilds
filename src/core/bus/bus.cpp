#include "bus.h"
#include "../arm/memory.h"

namespace Bus {

    template <Arm::AccessType accessType>
    u8 State::arm9Read8(Arm::State* arm, u32 addr, Arm::Access access) {
        int region = addr >> 24;
        switch (region) {
            // Main Memory
            case 2: {
                arm->addMainMemoryWaitstates9<accessType, Arm::AccessWidth::Bus16>(access);
                return mainRam[addr & 0x3fffff];
                break;
            }
            default:
                return 0;
        }
    }
    template u8 State::arm9Read8<Arm::AccessType::Code>(Arm::State*, u32 addr, Arm::Access access);
    template u8 State::arm9Read8<Arm::AccessType::Data>(Arm::State*, u32 addr, Arm::Access access);

}