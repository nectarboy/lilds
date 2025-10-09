#pragma once
#include "arm.h"

namespace Arm {

    template <AccessType type, AccessWidth width>
    inline void State::addMainMemoryWaitstates9(Access access) {
        if constexpr (type == AccessType::Code) {
            if constexpr (width == AccessWidth::Bus16)
                waitstates += 4 + evenClock;
            else
                waitstates += 9;
        }
        else {
            if constexpr (width == AccessWidth::Bus16)
                waitstates += access == Access::S ? 1 : 9;
            else
                waitstates += access == Access::S ? 2 : 10;
        }
    }

    template <AccessType type, AccessWidth width>
    inline void State::addSharedMemoryWaitstates9(Access access) {
        if constexpr (type == AccessType::Code) {
            if constexpr (width == AccessWidth::Bus16)
                waitstates += 2;
            else
                waitstates += 4;
        }
        else {
            waitstates += access == Access::S ? 1 : 4;
        }
    }

    template <AccessType type, AccessWidth width>
    inline void State::addVRAMWaitstates9(Access access) {
        if constexpr (type == AccessType::Code) {
            if constexpr (width == AccessWidth::Bus16)
                waitstates += 2 + evenClock;
            else
                waitstates += 5;
        }
        else {
            if constexpr (width == AccessWidth::Bus16)
                waitstates += access == Access::S ? 1 : 4;
            else
                waitstates += access == Access::S ? 2 : 5;
        }
    }

    template <AccessType type, AccessWidth width>
    inline void State::addTCMCacheWaitstates9(Access access) {
        waitstates += evenClock;
    }

}