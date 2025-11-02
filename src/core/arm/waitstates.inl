#pragma once
#include "arm.h"

namespace Arm {

    template <AccessType accessType>
    inline void State::addWaitstates(u32 cycles) {
        if (type == Type::Arm7)
            addWaitstates7<accessType>(cycles);
        else
            addWaitstates9<accessType>(cycles);
    }

    template <AccessType accessType>
    inline void State::addWaitstates9(u32 cycles) {
        if constexpr (accessType == AccessType::Code)
            codeWaitstates += cycles;
        else
            dataWaitstates += cycles;
    }

    template <AccessType accessType>
    inline void State::addWaitstates7(u32 cycles) {
        codeWaitstates += cycles;
    }

    template <AccessType type, AccessWidth width>
    inline void State::addMainMemoryWaitstates9(Access access) {
        if constexpr (type == AccessType::Code) {
            if constexpr (width == AccessWidth::Bus16)
                addWaitstates9<type>(4 + evenClock);
            else
                addWaitstates9<type>(9);
        }
        else {
            if constexpr (width == AccessWidth::Bus16)
                addWaitstates9<type>(access == Access::S ? 1 : 9);
            else
                addWaitstates9<type>(access == Access::S ? 2 : 10);
        }
    }

    template <AccessType type, AccessWidth width>
    inline void State::addSharedMemoryWaitstates9(Access access) {
        if constexpr (type == AccessType::Code) {
            if constexpr (width == AccessWidth::Bus16)
                addWaitstates9<type>(2);
            else
                addWaitstates9<type>(4);
        }
        else {
            addWaitstates9<type>(access == Access::S ? 1 : 4);
        }
    }

    template <AccessType type, AccessWidth width>
    inline void State::addVRAMWaitstates9(Access access) {
        if constexpr (type == AccessType::Code) {
            if constexpr (width == AccessWidth::Bus16)
                addWaitstates9<type>(2 + evenClock);
            else
                addWaitstates9<type>(5);
        }
        else {
            if constexpr (width == AccessWidth::Bus16)
                addWaitstates9<type>(access == Access::S ? 1 : 4);
            else
                addWaitstates9<type>(access == Access::S ? 2 : 5);
        }
    }

    template <AccessType type, AccessWidth width>
    inline void State::addTCMCacheWaitstates9(Access access) {
        addWaitstates9<type>(evenClock);
    }

}