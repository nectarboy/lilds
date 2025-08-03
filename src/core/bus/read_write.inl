#include "bus.h"
#include "io.inl"
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
            // IO9
            case 4: {
                if constexpr (!silent) {
                    if constexpr (is_same_v<T, u32>)
                        arm->addSharedMemoryWaitstates9<accessType, Arm::AccessWidth::Bus32>(access);
                    else
                        arm->addSharedMemoryWaitstates9<accessType, Arm::AccessWidth::Bus16>(access);
                }

                T val = io9Read8(addr);
                if constexpr (!is_same_v<T, u8>) {
                    val |= io9Read8(addr + 1) << 8;
                }
                if constexpr (is_same_v<T, u32>) {
                    val |= io9Read8(addr + 2) << 16;
                    val |= io9Read8(addr + 3) << 24;
                }
                printf("Arm9 read write %x \n", addr);
                return val;
                break;
            }
            default:
                printf("Arm9 reads %x \n", addr);
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
            // IO9
            case 4: {
                if constexpr (!silent) {
                    if constexpr (is_same_v<T, u32>)
                        arm->addSharedMemoryWaitstates9<accessType, Arm::AccessWidth::Bus32>(access);
                    else
                        arm->addSharedMemoryWaitstates9<accessType, Arm::AccessWidth::Bus16>(access);
                }

                io9Write8(addr, (u8)(val));
                if constexpr (!is_same_v<T, u8>) {
                    io9Write8(addr + 1, (u8)(val >> 8));
                }
                if constexpr (is_same_v<T, u32>) {
                    io9Write8(addr + 2, (u8)(val >> 16));
                    io9Write8(addr + 3, (u8)(val >> 24));
                }
                printf("Arm9 io write %x <- %x \n", addr, val);
                break;
            }
            // Vram
            case 6: {
                if constexpr (!silent) {
                    if constexpr (is_same_v<T, u32>)
                        arm->addVRAMWaitstates9<accessType, Arm::AccessWidth::Bus32>(access);
                    else
                        arm->addVRAMWaitstates9<accessType, Arm::AccessWidth::Bus16>(access);
                }

                if constexpr (is_same_v<T, u8>) // Arm9 8-bit writes to Vram are ignored
                    break;

                int pageId = getVramPageId(addr);
                VramPage* page = &vramPageTable[pageId];
                if (!page->empty) {
                    printf("Arm9 writes to vram %x <- %x \n", addr, val);
                    addr = page->pAddrBase + getVramPageOffset(addr);
                    if constexpr (is_same_v<T, u8>)
                        write8(vram, addr, val);
                    else if constexpr (is_same_v<T, u16>)
                        write16(vram, addr, val);
                    else
                        write32(vram, addr, val);
                }
                else {
                    printf("Arm9 writes to unmapped vram %x <- %x \n", addr, val);
                }
                break;
            }
            default:
                printf("Arm9 writes %x <- %x \n", addr, val);
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
                printf("Arm7 reads %x \n", addr);
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
                printf("Arm7 writes %x <- %x \n", addr, val);
                break;
        }
    }

}