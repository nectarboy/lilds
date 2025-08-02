#include "bus.h"
#include "../arm/waitstates.inl"

namespace Bus {
    using namespace std;

    // arm 9 methods
    template <typename T, Arm::AccessType accessType, bool silent>
    inline T State::arm9Read(Arm::State* arm, u32 addr, Arm::Access access) {
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
                printf("Arm9 read from region %d \n", region);
                return 0;
        }
    }

    template <typename T, Arm::AccessType accessType, bool silent>
    inline void State::arm9Write(Arm::State* arm, u32 addr, T val, Arm::Access access) {
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
                printf("Arm9 write to region %d \n", region);
                break;
        }
    }



    // arm7 methods
    template <typename T, Arm::AccessType accessType, bool silent>
    inline T State::arm7Read(Arm::State* arm, u32 addr, Arm::Access access) {
        if constexpr (is_same_v<T, u32>)
            addr &= 0xffff'fffc;
        else if constexpr (is_same_v<T, u16>)
            addr &= 0xffff'fffe;

        int region = addr >> 24;
        switch (region) {
            // Main Memory
            case 2: {
                if constexpr (!silent) {
                    // if constexpr (is_same_v<T, u32>)
                    //     arm->addMainMemoryWaitstates7<accessType, Arm::AccessWidth::Bus32>(access);
                    // else
                    //     arm->addMainMemoryWaitstates7<accessType, Arm::AccessWidth::Bus16>(access);
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
                printf("Arm7 read from region %d \n", region);
                return 0;
        }
    }

    template <typename T, Arm::AccessType accessType, bool silent>
    inline void State::arm7Write(Arm::State* arm, u32 addr, T val, Arm::Access access) {
        if constexpr (is_same_v<T, u32>)
            addr &= 0xffff'fffc;
        else if constexpr (is_same_v<T, u16>)
            addr &= 0xffff'fffe;

        int region = addr >> 24;
        switch (region) {
            // Main Memory
            case 2: {
                if constexpr (!silent) {
                    // if constexpr (is_same_v<T, u32>)
                    //     arm->addMainMemoryWaitstates7<accessType, Arm::AccessWidth::Bus32>(access);
                    // else
                    //     arm->addMainMemoryWaitstates7<accessType, Arm::AccessWidth::Bus16>(access);
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
                printf("Arm7 write to region %d \n", region);
                break;
        }
    }

}