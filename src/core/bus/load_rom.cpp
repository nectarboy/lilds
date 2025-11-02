#include "../../helpers.h"
#include "bus.h"
#include "read_write.inl"
#include "../arm/arm.h"

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
        if (rom) delete[] rom;
        rom = new u8[romSize];
        for (u32 i = 0; i < romSize; i++) {
            if (i < fileSize)
                rom[i] = u8(romFile.data()[i]);
            else
                rom[i] = 0;
        }

        std::cout << "Rom size rounded up:\t" << romSize << " bytes (" << romSize / 1024 << " kb)\n";
    }

    void State::sideLoadRomToMainMem(Arm::State* arm7, Arm::State* arm9) {
        u32 romOff9 = read32(rom, 0x20);
        u32 entryAddr9 = read32(rom, 0x24);
        u32 ramAddr9 = read32(rom, 0x28);
        u32 size9 = read32(rom, 0x2c);
        // printf("romoff9: %zx ramaddr9: %zx size9: %zx \n", romOff9, ramAddr9, size9);
        for (int i = 0; i < size9; i++) {
            // printf("writing to: %zx \n", ramAddr9 + i - 0x0200'0000);
            arm9Write<u8, Arm::AccessType::Data, true>(nullptr, ramAddr9 + i, rom[romOff9 + i], Arm::Access::N);
        }
        arm9->sideLoadAt(entryAddr9);

        u32 romOff7 = read32(rom, 0x30);
        u32 entryAddr7 = read32(rom, 0x34);
        u32 ramAddr7 = read32(rom, 0x38);
        u32 size7 = read32(rom, 0x3c);
        // printf("romoff7: %zx ramaddr7: %zx size7: %zx \n", romOff7, ramAddr7, size7);
        for (u32 i = 0; i < size7; i++) {
            // printf("writing to: %zx \n", ramAddr7 + i - 0x0200'0000);
            arm7Write<u8, Arm::AccessType::Data, true>(nullptr, ramAddr7 + i, rom[romOff7 + i], Arm::Access::N);
        }
        arm7->sideLoadAt(entryAddr7);

        printf("arm9 entry: 0x%X \narm7 entry: 0x%X \n", entryAddr9, entryAddr7);
    }

}