#include "../../helpers.h"
#include "bus.h"

namespace Bus {

    void State::loadRomFile(std::vector<char>& romFile) {
        u64 fileSize = romFile.size();
        romSize = 0;

        // Clamp to a max (max 4GB?)
        if (fileSize >= 0x1'0000'0000) {
            fileSize = 0x1'0000'0000;
            romSize = fileSize;
        }
        // Round up size to a power of two
        else {
            while (romSize < fileSize) {
                romSize <<= 1;
                romSize |= 1;
            }
            romSize += 1;
        }

        // Create rom array
        if (rom)
            delete[] rom;
        rom = new u8[romSize];
        for (u32 i = 0; i < romSize; i++) {
            if (i < fileSize)
                rom[i] = u8(romFile.data()[i]);
            else
                rom[i] = 0;
        }

        std::cout << "Rom size rounded up:\t" << romSize << " bytes (" << romSize / 1024 << " kb)\n";
    }

    void State::sideLoadRomToMainMem() {
        u32 arm9RomOff = read32(rom, 0x20);
        u32 arm9RamAddr = read32(rom, 0x28);
        u32 arm9Size = read32(rom, 0x2c);
        for (int i = 0; i < arm9Size; i++)
            arm9Write<u8, NULL, true>(NULL, arm9RamAddr + i, rom[arm9RomOff + i], NULL);
    }

}