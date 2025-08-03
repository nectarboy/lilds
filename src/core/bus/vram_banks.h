namespace Bus {

    // TODO: handle overlapping banks
    struct VramPage {
        bool empty = true;
        bool overlapping = false;
        bool banksSet[9];
        int pAddrBase = 0;

        void setBank(uint bank) {
            assert(bank <= 9);
            banksSet[bank] = true;
            if (empty)
                empty = false;
            else
                overlapping = true;
        }

        void unsetBank(uint bank) {
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