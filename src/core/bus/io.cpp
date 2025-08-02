#include "bus.h"
#include "io.h"

namespace Bus {

    u8 IO::arm9Read8(u32 addr) {
        printf("Arm9 io read %x \n", addr);
        return 0;
    }
    void IO::arm9Write8(u32 addr, u8 val) {
        printf("Arm9 io write %x <- %x \n", addr, val);
    }

    u8 IO::arm7Read8(u32 addr) {
        printf("Arm9 io read %x \n", addr);
        return 0;
    }
    void IO::arm7Write8(u32 addr, u8 val) {
        printf("Arm7 io write %x <- %x \n", addr, val);
    }

    void IO::initialize() {

    }

}