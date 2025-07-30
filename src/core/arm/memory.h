#pragma once
#include "arm.h"
#include "../bus/bus.h"

namespace Arm {

    // memory methods (TODO: inline the bus methods for extra perf)
    inline u8 State::read8(u32 addr, Access access) {
        Bus::State* bus = static_cast<Bus::State*>(ds->bus);
        return bus->arm9Read<u8, AccessType::Data>(this, addr, access);
    }
    inline u16 State::read16(u32 addr, Access access) {
        Bus::State* bus = static_cast<Bus::State*>(ds->bus);
        return bus->arm9Read<u16, AccessType::Data>(this, addr, access);
    }
    inline u32 State::read32(u32 addr, Access access) {
        Bus::State* bus = static_cast<Bus::State*>(ds->bus);
        return bus->arm9Read<u32, AccessType::Data>(this, addr, access);
    }
    inline void State::write8(u32 addr, u8 val, Access access) {
        Bus::State* bus = static_cast<Bus::State*>(ds->bus);
        bus->arm9Write<u8, AccessType::Data>(this, addr, val, access);
    }
    inline void State::write16(u32 addr, u16 val, Access access) {
        Bus::State* bus = static_cast<Bus::State*>(ds->bus);
        bus->arm9Write<u16, AccessType::Data>(this, addr, val, access);
    }
    inline void State::write32(u32 addr, u32 val, Access access) {
        Bus::State* bus = static_cast<Bus::State*>(ds->bus);
        bus->arm9Write<u32, AccessType::Data>(this, addr, val, access);
    }
    inline u16 State::readCode16(u32 addr, Access access) {
        Bus::State* bus = static_cast<Bus::State*>(ds->bus);
        return bus->arm9Read<u16, AccessType::Code>(this, addr, access);
    }
    inline u32 State::readCode32(u32 addr, Access access) {
        Bus::State* bus = static_cast<Bus::State*>(ds->bus);
        return bus->arm9Read<u32, AccessType::Code>(this, addr, access);
    }

    // waitstate methods
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