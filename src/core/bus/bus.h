#pragma once
#include "../../helpers.h"
#include "../arm/arm.h"

namespace DS {
    struct State;
}

namespace Bus {

    struct State {
        State() {}

        // references
        DS::State* ds = nullptr;

        // memory regions
        u8 mainRam[0x400000];
        u8 sharedRam[98304];
        u8 vram[671744];

        // memory methods (TODO: 16 and 32 bit reads that dont call 8 reads, is possible probably)
        u8 arm7Read8(Arm::State* arm, u32 addr, Arm::Access access);
        void arm7Write8(Arm::State* arm, u32 addr, u32 val, Arm::Access access);
        
        template <Arm::AccessType accessType>
        u8 arm9Read8(Arm::State* arm, u32 addr, Arm::Access access);
        void arm9Write8(Arm::State* arm, u32 addr, u32 val, Arm::Access access);
    };

}