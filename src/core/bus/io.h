#pragma once
#include "../../helpers.h"

namespace Bus {

    // namespace {
        struct IO {

            // read/write methods
            u8 arm9Read8(u32 addr);
            void arm9Write8(u32 addr, u8 val);
            u8 arm7Read8(u32 addr);
            void arm7Write8(u32 addr, u8 val);

            // initalization methods
            void initialize();

            // registers

        };
    // }

}