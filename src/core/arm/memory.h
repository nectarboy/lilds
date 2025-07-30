#pragma once
#include "arm.h"
#include "../bus/bus.h"

namespace Arm {

    // memory methods
    inline u8 State::read8(u32 addr, Access access) {
        Bus::State* bus = static_cast<Bus::State*>(ds->bus);
        u8 val = bus->arm9Read8<AccessType::Data>(this, addr, access);
        return val;
    }
    inline u16 State::read16(u32 addr, Access access) {
        return 0;
    }
    inline u32 State::read32(u32 addr, Access access) {
        return 0;
    }
    inline void State::write8(u32 addr, u8 val, Access access) {}
    inline void State::write16(u32 addr, u16 val, Access access) {}
    inline void State::write32(u32 addr, u32 val, Access access) {}

    // waitstate methods
    template <AccessType type, AccessWidth width>
    inline void State::addMainMemoryWaitstates(Access access) {
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
    template void State::addMainMemoryWaitstates<AccessType::Code, AccessWidth::Bus16>(Access access);
    template void State::addMainMemoryWaitstates<AccessType::Code, AccessWidth::Bus32>(Access access);
    template void State::addMainMemoryWaitstates<AccessType::Data, AccessWidth::Bus16>(Access access);
    template void State::addMainMemoryWaitstates<AccessType::Data, AccessWidth::Bus32>(Access access);

}