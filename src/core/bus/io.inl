#include "bus.h"
#include "../joypad/joypad.h"

#define IO_DISPCNT      0x4000000
#define IO_DISPSTAT     0x4000004
#define IO_KEYINPUT     0x4000130
#define IO_EXTKEYIN     0x4000136 
#define IO_IME          0x4000208
#define IO_VRAMSTAT     0x4000240
#define IO_VRAMCNT_A    0x4000240
#define IO_VRAMCNT_B    0x4000241
#define IO_POWCNT       0x4000304

namespace Bus {

    u8 State::io9Read8(u32 addr) {
        switch (addr) {

            case IO_DISPSTAT:
                return (test++ & 0xfff) == 0;
                break;

            case IO_KEYINPUT:
                return static_cast<Joypad::State*>(ds->joypad)->getIOPart_KeyInput() & 0xff;
                break;
            case IO_KEYINPUT + 1:
                return static_cast<Joypad::State*>(ds->joypad)->getIOPart_KeyInput() >> 8;
                break;
            
        }
        return 0;
    }
    void State::io9Write8(u32 addr, u8 val) {
        switch (addr) {

            case IO_VRAMCNT_A:
                setVramCntAB(0, val);
                break;
            case IO_VRAMCNT_B:
                setVramCntAB(1, val);
                break;

        }
    }

    u8 State::io7Read8(u32 addr) {
        return 0;
    }
    void State::io7Write8(u32 addr, u8 val) {
    }

}