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

    template <typename T, Arm::AccessType accessType>
    T State::arm9Read(Arm::State* arm, u32 addr, Arm::Access access) {
        if constexpr (std::is_same_v<T, u32>)
            addr &= 0xffff'fffc;
        else if constexpr (std::is_same_v<T, u16>)
            addr &= 0xffff'fffe;

        int region = addr >> 24;
        switch (region) {
            // Main Memory
            case 2: {
                if constexpr (std::is_same_v<T, u32>)
                    arm->addMainMemoryWaitstates9<accessType, Arm::AccessWidth::Bus32>(access);
                else
                    arm->addMainMemoryWaitstates9<accessType, Arm::AccessWidth::Bus16>(access);

                addr &= 0x3fffff;
                if constexpr (std::is_same_v<T, u8>)
                    return mainRam[addr];
                else if constexpr (std::is_same_v<T, u16>)
                    return mainRam[addr] | (mainRam[addr+1] << 8);
                else
                    return mainRam[addr] | (mainRam[addr+1] << 8) | (mainRam[addr+2] << 16) | (mainRam[addr+3] << 24);
                break;
            }
            default:
                return 0;
        }
    }
    template u8 State::arm9Read<u8, Arm::AccessType::Code>(Arm::State*, u32 addr, Arm::Access access);
    template u8 State::arm9Read<u8, Arm::AccessType::Data>(Arm::State*, u32 addr, Arm::Access access);
    template u16 State::arm9Read<u16, Arm::AccessType::Code>(Arm::State*, u32 addr, Arm::Access access);
    template u16 State::arm9Read<u16, Arm::AccessType::Data>(Arm::State*, u32 addr, Arm::Access access);
    template u32 State::arm9Read<u32, Arm::AccessType::Code>(Arm::State*, u32 addr, Arm::Access access);
    template u32 State::arm9Read<u32, Arm::AccessType::Data>(Arm::State*, u32 addr, Arm::Access access);

}