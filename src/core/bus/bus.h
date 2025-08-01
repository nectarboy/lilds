#pragma once
#include "../../helpers.h"
#include "../arm/arm.h"

namespace DS {
    struct State;
}

namespace Bus {

    // array read/write methods
    inline u8 read8(const u8 arr[], const u32 addr) {
        return arr[addr];
    }
    inline u16 read16(const u8 arr[], const u32 addr) {
        return arr[addr] | (arr[addr + 1] << 8);
    }
    inline u32 read32(const u8 arr[], const u32 addr) {
        return arr[addr] | (arr[addr + 1] << 8) | (arr[addr + 2] << 16) | (arr[addr + 3] << 24);
    }
    inline void write8(u8 arr[], const u8 val, const u32 addr) {
        arr[addr] = val;
    }
    inline void write16(u8 arr[], const u16 val, const u32 addr) {
        arr[addr] = (u8)val;
        arr[addr + 1] = (u8)(val >> 8);
    }
    inline void write32(u8 arr[], const u32 val, const u32 addr) {
        arr[addr] = (u8)val;
        arr[addr + 1] = (u8)(val >> 8);
        arr[addr + 2] = (u8)(val >> 16);
        arr[addr + 3] = (u8)(val >> 24);
    }

    struct State {
        State() {}

        // references
        DS::State* ds = nullptr;

        // memory regions
        u8 mainRam[0x400000];
        u8 sharedRam[98304];
        u8 vram[671744];
        u8* rom = new u8[0];
        u64 romSize = 0;

        // memory methods (TODO: 16 and 32 bit reads that dont call 8 reads, is possible probably)
        u8 arm7Read8(Arm::State* arm, u32 addr, Arm::Access access);
        void arm7Write8(Arm::State* arm, u32 addr, u32 val, Arm::Access access);

        template <Arm::AccessType accessType>
        u8 arm9Read8(Arm::State* arm, u32 addr, Arm::Access access);
        void arm9Write8(Arm::State* arm, u32 addr, u32 val, Arm::Access access);


        template <typename T, Arm::AccessType accessType, bool silent = false>
        T arm7Read(Arm::State* arm, u32 addr, Arm::Access access);
        template <typename T, Arm::AccessType accessType, bool silent = false>
        T arm7Write(Arm::State* arm, u32 addr, T val, Arm::Access access);

        template <typename T, Arm::AccessType accessType, bool silent = false>
        T arm9Read(Arm::State* arm, u32 addr, Arm::Access access);
        template <typename T, Arm::AccessType accessType, bool silent = false>
        void arm9Write(Arm::State* arm, u32 addr, T val, Arm::Access access);

        // rom loading methods
        void loadRomFile(std::vector<char>& romFile);
        void sideLoadRomToMainMem();
    };

}