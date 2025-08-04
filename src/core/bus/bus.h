#pragma once
#include "../../helpers.h"
#include "../arm/arm.h"
#include "vram_banks.h"

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
    inline void write8(u8 arr[], const u32 addr, const u8 val) {
        arr[addr] = val;
    }
    inline void write16(u8 arr[], const u32 addr, const u16 val) {
        arr[addr] = (u8)val;
        arr[addr + 1] = (u8)(val >> 8);
    }
    inline void write32(u8 arr[], const u32 addr, const u32 val) {
        arr[addr] = (u8)val;
        arr[addr + 1] = (u8)(val >> 8);
        arr[addr + 2] = (u8)(val >> 16);
        arr[addr + 3] = (u8)(val >> 24);
    }

    struct State {
        State() {}

        // references
        DS::State* ds = nullptr;

        // initialization methods
        void initialize();

        // memory regions
        u8 iTcm[0x8000];
        u8 dTcm[0x4000];
        u8 mainRam[0x400000];
        u8 sharedRam[0x18000];
        u8 vram[0xA4000];
        u8* rom = new u8[0];
        u64 romSize = 0;
            
        // read/write methods
        template <typename T, Arm::AccessType accessType, bool silent = false>
        inline T arm9Read(Arm::State* arm, u32 addr, Arm::Access access);
        template <typename T, Arm::AccessType accessType, bool silent = false>
        inline void arm9Write(Arm::State* arm, u32 addr, T val, Arm::Access access);

        template <typename T, Arm::AccessType accessType, bool silent = false>
        inline T arm7Read(Arm::State* arm, u32 addr, Arm::Access access);
        template <typename T, Arm::AccessType accessType, bool silent = false>
        inline void arm7Write(Arm::State* arm, u32 addr, T val, Arm::Access access);

        // io methods
        inline u8 io9Read8(u32 addr);
        inline void io9Write8(u32 addr, u8 val);
        inline u8 io7Read8(u32 addr);
        inline void io7Write8(u32 addr, u8 val);

        // rom loading methods
        void loadRomFile(std::vector<char>& romFile);
        void sideLoadRomToMainMem(Arm::State* arm7, Arm::State* arm9);

        // vram banks
        static const u32 vramBankStartPAddrLut[9];
        VramPage vramPageTable[1024]; // ~0x8A4000~ 0x1000000 possible virtual space / 16KB (16KB per page)
        VramCnt vramCnt[9];
        void setVramCntAB(int bank, u8 newCnt);
        void setVramCntCD(int bank, u8 newCnt);
        void fixNonemptyOldPage(VramPage* page);
        uint getVramPageId(u32 addr);
        uint getVramPageOffset(u32 addr);

    };

}