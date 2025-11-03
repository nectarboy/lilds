namespace Bus {

    // TODO: handle overlapping banks
    struct VramPage {
        bool empty = true;
        bool overlapping = false;
        bool banksSet[9]; // Consider making this a bitfield
        int pAddrBase = 0;

        inline void setBank(uint bank) {
            assert(bank <= 9);
            if (empty)
                empty = false;
            else
                overlapping |= !banksSet[bank];
            banksSet[bank] = true;
        }

        inline void unsetBank(uint bank) {
            assert(bank <= 9);
            banksSet[bank] = false;

            if (!empty && overlapping) {
                overlapping = false;
                for (int i = 0; i < 9; i ++) {
                    if (banksSet[i]) {
                        overlapping = true;
                        break;
                    }
                }
            }
            else if (!empty) {
                empty = true;
            }
        }
    };

    struct VramCnt {
        u8 val = 0;
        int pageId = 0;
    };
}