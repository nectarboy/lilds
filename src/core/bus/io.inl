#include "bus.h"

#define IO_DISPCNT     0x4000000
#define IO_DISPSTAT    0x4000004
#define IO_IME         0x4000208
#define IO_VRAMSTAT    0x4000240
#define IO_VRAMCNT_A   0x4000240
#define IO_VRAMCNT_B   0x4000241
#define IO_POWCNT      0x4000304

namespace Bus {

    u8 State::io9Read8(u32 addr) {
        switch (addr) {
            case IO_DISPSTAT:
                return rand() & 1;
                break;
        }
        return 0;
    }
    void State::io9Write8(u32 addr, u8 val) {
        switch (addr) {
            case IO_VRAMCNT_A:
                setVramCntA(val);
                break;
            case IO_VRAMCNT_B:
                // setVramCntB(val);
                break;
        }
    }

    u8 State::io7Read8(u32 addr) {
        return 0;
    }
    void State::io7Write8(u32 addr, u8 val) {
    }

}