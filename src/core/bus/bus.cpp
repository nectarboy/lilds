#include "bus.h"
#include "../arm/memory.inl"

namespace Bus {
    using namespace std;

    template <typename T, Arm::AccessType accessType, bool silent = false>
    T State::arm9Read(Arm::State* arm, u32 addr, Arm::Access access) {
        if constexpr (is_same_v<T, u32>)
            addr &= 0xffff'fffc;
        else if constexpr (is_same_v<T, u16>)
            addr &= 0xffff'fffe;

        int region = addr >> 24;
        switch (region) {
            // Main Memory
            case 2: {
                if constexpr (!silent) {
                    if constexpr (is_same_v<T, u32>)
                        arm->addMainMemoryWaitstates9<accessType, Arm::AccessWidth::Bus32>(access);
                    else
                        arm->addMainMemoryWaitstates9<accessType, Arm::AccessWidth::Bus16>(access);
                }

                addr &= 0x3fffff;
                if constexpr (is_same_v<T, u8>)
                    return read8(mainRam, addr);
                else if constexpr (is_same_v<T, u16>)
                    return read16(mainRam, addr);
                else
                    return read32(mainRam, addr);
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

    template <typename T, Arm::AccessType accessType, bool silent = false>
    void State::arm9Write(Arm::State* arm, u32 addr, T val, Arm::Access access) {
        if constexpr (is_same_v<T, u32>)
            addr &= 0xffff'fffc;
        else if constexpr (is_same_v<T, u16>)
            addr &= 0xffff'fffe;

        int region = addr >> 24;
        switch (region) {
            // Main Memory
            case 2: {
                if constexpr (!silent) {
                    if constexpr (is_same_v<T, u32>)
                        arm->addMainMemoryWaitstates9<accessType, Arm::AccessWidth::Bus32>(access);
                    else
                        arm->addMainMemoryWaitstates9<accessType, Arm::AccessWidth::Bus16>(access);
                }

                addr &= 0x3fffff;
                if constexpr (is_same_v<T, u8>)
                    write8(mainRam, addr, val);
                else if constexpr (is_same_v<T, u16>)
                    write16(mainRam, addr, val);
                else
                    write32(mainRam, addr, val);
                break;
            }
            default:
                break;
        }
    }
    template void State::arm9Write<u8, Arm::AccessType::Code>(Arm::State*, u32 addr, u8 val, Arm::Access access);
    template void State::arm9Write<u8, Arm::AccessType::Data>(Arm::State*, u32 addr, u8 val, Arm::Access access);
    template void State::arm9Write<u16, Arm::AccessType::Code>(Arm::State*, u32 addr, u16 val, Arm::Access access);
    template void State::arm9Write<u16, Arm::AccessType::Data>(Arm::State*, u32 addr, u16 val, Arm::Access access);
    template void State::arm9Write<u32, Arm::AccessType::Code>(Arm::State*, u32 addr, u32 val, Arm::Access access);
    template void State::arm9Write<u32, Arm::AccessType::Data>(Arm::State*, u32 addr, u32 val, Arm::Access access);

}