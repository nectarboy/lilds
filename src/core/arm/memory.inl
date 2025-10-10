#pragma once
#include "arm.h"
#include "../bus/bus.h"
#include "../bus/read_write.inl"

namespace Arm {

    inline u8 State::read8(u32 addr, Access access) {
        Bus::State* bus = static_cast<Bus::State*>(ds->bus);
        if (type == Type::Arm7)
            return bus->arm7Read<u8, AccessType::Data, false>(this, addr, access);
        else
            return bus->arm9Read<u8, AccessType::Data, false>(this, addr, access);
    }
    inline u16 State::read16(u32 addr, Access access) {
        Bus::State* bus = static_cast<Bus::State*>(ds->bus);
        if (type == Type::Arm7)
            return bus->arm7Read<u16, AccessType::Data, false>(this, addr, access);
        else
            return bus->arm9Read<u16, AccessType::Data>(this, addr, access);
    }
    inline u32 State::read32(u32 addr, Access access) {
        Bus::State* bus = static_cast<Bus::State*>(ds->bus);
        if (type == Type::Arm7)
            return bus->arm7Read<u32, AccessType::Data, false>(this, addr, access);
        else
            return bus->arm9Read<u32, AccessType::Data>(this, addr, access);
    }
    inline void State::write8(u32 addr, u8 val, Access access) {
        Bus::State* bus = static_cast<Bus::State*>(ds->bus);
        if (type == Type::Arm7)
            bus->arm7Write<u8, AccessType::Data>(this, addr, val, access);
        else
            bus->arm9Write<u8, AccessType::Data>(this, addr, val, access);
    }
    inline void State::write16(u32 addr, u16 val, Access access) {
        Bus::State* bus = static_cast<Bus::State*>(ds->bus);
        if (type == Type::Arm7)
            bus->arm9Write<u16, AccessType::Data>(this, addr, val, access);
        else
            bus->arm9Write<u16, AccessType::Data>(this, addr, val, access);
    }
    inline void State::write32(u32 addr, u32 val, Access access) {
        Bus::State* bus = static_cast<Bus::State*>(ds->bus);
        if (type == Type::Arm7)
            bus->arm9Write<u32, AccessType::Data>(this, addr, val, access);
        else
            bus->arm9Write<u32, AccessType::Data>(this, addr, val, access);
    }
    inline u16 State::readCode16(u32 addr, Access access) {
        Bus::State* bus = static_cast<Bus::State*>(ds->bus);
        if (type == Type::Arm7)
            return bus->arm7Read<u16, AccessType::Code, false>(this, addr, access);
        else
            return bus->arm9Read<u16, AccessType::Code>(this, addr, access);
    }
    inline u32 State::readCode32(u32 addr, Access access) {
        Bus::State* bus = static_cast<Bus::State*>(ds->bus);
        if (type == Type::Arm7)
            return bus->arm7Read<u32, AccessType::Code, false>(this, addr, access);
        else
            return bus->arm9Read<u32, AccessType::Code>(this, addr, access);
    }

}