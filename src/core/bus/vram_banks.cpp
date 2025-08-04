#include "bus.h"

namespace Bus {

    const u32 State::vramBankStartPAddrLut[] = {
        0x00000, // A
        0x20000, // B
        0x40000, // C
        0x60000, // D
        0x80000, // E
        0x90000, // F
        0x94000, // G
        0x98000, // H
        0xA0000, // I
    };

    void State::setVramCntAB(int bank, u8 newCnt) {
        static const int numOfPages = 8;
        const int pAddrStart = vramBankStartPAddrLut[bank];
        VramCnt* cnt = &vramCnt[bank];

        newCnt &= 0b11011001;
        if (newCnt == cnt->val)
            return;

        int newMst = newCnt & 3;
        int newOfs = (newCnt >> 3) & 3;
        bool shouldSetNewPage = newCnt >> 7;

        int startAddr;
        switch (newMst) {
            case 0:
                startAddr = 0x6800000 + pAddrStart;
                break;
            case 1:
                startAddr = 0x6000000 + 0x20000 * newOfs;
                break;
            case 2:
                startAddr = 0x6400000 + 0x20000 * (newOfs & 1);
                break;
            case 3:
                shouldSetNewPage = false;
                // 3D stuff, bank is not mapped to Arm9 space here
                break;
            default:
                lilds__unreachable();
        }
        int oldPageStart = cnt->pageId;
        int newPageStart = getVramPageId(startAddr);
        bool oldPageWasSet = (cnt->val >> 7) & ((cnt->val & 3) != 3);

        for (int i = 0; i < numOfPages; i++) {
            VramPage* oldPage = &vramPageTable[oldPageStart + i];
            VramPage* newPage = &vramPageTable[newPageStart + i];
            if (oldPageWasSet) {
                oldPage->unsetBank(bank);

                // If old page still has a previous bank occupying it, fix its pAddr
                if (!oldPage->empty && !oldPage->overlapping)
                    fixNonemptyOldPage(oldPage);
            }

            if (shouldSetNewPage) {
                newPage->setBank(bank);
                newPage->pAddrBase = pAddrStart + 0x4000*i;
                printf("Vram Bank %x set page: %x pAddr: %x \n", bank, newPageStart + i, pAddrStart + 0x4000*i);
            }
        }

        cnt->pageId = newPageStart;
        cnt->val = newCnt;
    }

    void State::setVramCntCD(int bank, u8 newCnt) {
        static const int numOfPages = 8;
        const int pAddrStart = vramBankStartPAddrLut[bank];
        VramCnt* cnt = &vramCnt[bank];

        newCnt &= 0b11111001;
        if (newCnt == cnt->val)
            return;

        int newMst = newCnt & 3;
        int newOfs = (newCnt >> 3) & 3;
        bool shouldSetNewPage = newCnt >> 7;

        int startAddr;
        switch (newMst) {
            case 0:
                startAddr = 0x6800000 + pAddrStart;
                break;
            case 1:
                startAddr = 0x6000000 + 0x20000 * newOfs;
                break;
            case 2:
                startAddr = bank == 2 ? 0x6200000 : 0x6600000;
                break;
            case 3:
                shouldSetNewPage = false;
                // 3D stuff, bank is not mapped to Arm9 space here
                break;
            case 4:
                startAddr = 0x6000000 + 0x20000 * (newOfs & 1);
                break;
            default:
                printf("Weird VramCntCD MST %x \n", newMst);
                lilds__crash();
        }
        int oldPageStart = cnt->pageId;
        int newPageStart = getVramPageId(startAddr);
        bool oldPageWasSet = (cnt->val >> 7) & ((cnt->val & 3) != 3);

        for (int i = 0; i < numOfPages; i++) {
            VramPage* oldPage = &vramPageTable[oldPageStart + i];
            VramPage* newPage = &vramPageTable[newPageStart + i];
            if (oldPageWasSet) {
                oldPage->unsetBank(bank);

                // If old page still has a previous bank occupying it, fix its pAddr
                if (!oldPage->empty && !oldPage->overlapping)
                    fixNonemptyOldPage(oldPage);
            }

            if (shouldSetNewPage) {
                newPage->setBank(bank);
                newPage->pAddrBase = pAddrStart + 0x4000*i;
                printf("Vram Bank %x set page: %x pAddr: %x \n", bank, newPageStart + i, pAddrStart + 0x4000*i);
            }
        }

        cnt->pageId = newPageStart;
        cnt->val = newCnt;
    }

    void State::fixNonemptyOldPage(VramPage* page) {
        int bank;
        for (bank = 0; bank < 9; bank++)
            if (page->banksSet[bank])
                break;

        // Set the bank's page depending on its VramCnt.val (without unsetting any other pages)
        // ...
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