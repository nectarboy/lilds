#include "bus.h"

namespace Bus {

    void State::setVramCntA(u8 newCnt) {
        static const int numOfPages = 32;
        static const int pAddrStart = 0;
        VramCnt* cnt = &vramCnt[0];

        newCnt &= 0b11011001;
        if (newCnt == cnt->val)
            return;

        int newMst = newCnt & 3;
        int newOfs = (newCnt >> 3) & 3;
        bool newEnabled = newCnt >> 7;

        int startAddr;
        switch (newMst) {
            case 0:
                startAddr = 0x6800000;
                break;
            case 1:
                startAddr = 0x6000000 + 0x20000 * newOfs;
                break;
            case 2:
                startAddr = 0x6400000 + 0x20000 * (newOfs & 1);
                break;
            case 3:
                // ???
                break;
            default:
                lilds__unreachable();
        }
        int oldPageStart = cnt->pageId;
        int newPageStart = getVramPageId(startAddr);

        for (int i = 0; i < numOfPages; i++) {
            VramPage* oldPage = &vramPageTable[oldPageStart + i];
            VramPage* newPage = &vramPageTable[newPageStart + i];

            oldPage->unsetBank(0);

            // If old page still has a previous bank occupying it, fix its pAddr
            if (!oldPage->empty && !oldPage->overlapping)
                fixNonemptyOldPage(oldPage);

            if (newEnabled) {
                newPage->setBank(0);
                newPage->pAddrBase = pAddrStart + 0x4000*i;
                printf("Vram Bank A set page: %x pAddr: %x \n", newPageStart + i, pAddrStart + 0x4000*i);
            }
        }

        cnt->pageId = newPageStart;
        cnt->val = newCnt;
    }

    void State::fixNonemptyOldPage(VramPage* oldPage) {

    }

    uint State::getVramPageId(u32 addr) {
        uint id = (addr & 0x00ff'ffff) >> 14;
        assert(id < 1024);
        return id;
    }
    uint State::getVramPageOffset(u32 addr) {
        return addr & 0x3fff;
    }

}