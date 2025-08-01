#include "../../../helpers.h"
#include "../arm.h"
#include "interpreter.h"

namespace Interpreter {
    using namespace Arm;

    namespace Arm {
        ArmInstruction decode(State* cpu, u32 instruction) {
            switch ((instruction >> 26) & 0b11) {
                case 0b00: {
                    u32 bits543210 = (instruction >> 20) & 0b111111; // TODO: Remove the and later
                    u32 bits7654 = (instruction >> 4) & 0b1111;

                    // TODO: can optimize common cases for bits7654, just make sure it works first
                    if ((bits543210 & 0b111100) == 0b000000 && bits7654 == 0b1001) {// Needs arguments to be rewritten
                        if (cpu->canPrint()) std::cout << "Multiply:\t\t" << std::hex << instruction << std::dec << "\n";
                        return &mul;
                    }
                    if ((bits543210 & 0b111000) == 0b001000 && bits7654 == 0b1001) {// Needs to be implemented
                        if (cpu->canPrint()) std::cout << "Multiply Long:\t\t" << std::hex << instruction << std::dec << "\n";
                        return &mull;
                    }
                    if ((bits543210 & 0b111011) == 0b010000 && bits7654 == 0b1001) { // ^^^
                        if (cpu->canPrint()) std::cout << "Single Data Swap:\t" << std::hex << instruction << std::dec << "\n";
                        cpu->PRINTSTATE();
                        return &swp;
                    }
                    if ((bits543210 & 0b100000) == 0b000000 && (bits7654 & 0b1001) == 0b1001) {
                        if (cpu->canPrint()) std::cout << "HW/S Data Transfer:\t" << std::hex << instruction << std::dec << "\n";
                        return &ldrh_strh;
                    }
                    if ((bits543210 & 0b111011) == 0b010000 && bits7654 == 0b0000) {
                        if (cpu->canPrint()) std::cout << "MRS:\t" << std::hex << instruction << std::dec << "\n";
                        return &mrs;
                    }
                    if (
                        ((bits543210 & 0b111011) == 0b010010 && bits7654 == 0b0000) ||  // Register ; TODO: optimize this fuckass thing here
                        ((bits543210 & 0b111011) == 0b110010)                           // Immediate
                    ) {
                        if (cpu->canPrint()) std::cout << "MSR:\t" << std::hex << instruction << std::dec << "\n";
                        return &msr<false>;
                    }
                    if ((bits543210 & 0b111111) == 0b010010 && bits7654 == 0b0001) {
                        if (cpu->canPrint()) std::cout << "Branch and Exchange:\t" << std::hex << instruction << std::dec << "\n";
                        return &bx<false>;
                    }
                    if ((bits543210 & 0b111011) == 0b110000) {
                        std::cout << "Invalid instruction! \nins: " << std::hex << instruction << "\nbits543210: " << std::dec << std::bitset<6>(bits543210) << "\tbits7654: " << std::bitset<4>(bits7654) << "\n\n";
                        cpu->PRINTSTATE();
                    }
                    if (cpu->canPrint()) std::cout << "Data Processing:\t" << std::hex << instruction << std::dec << "\n";
                    return &alu<false>;
                    break;
                }
                case 0b01: {
                    u32 bits543210 = (instruction >> 20) & 0b111111;
                    u32 bits7654 = (instruction >> 4) & 0b1111;

                    if ((bits543210 & 0b100000) == 0b100000 && (bits7654 & 1) == 1) {
                        std::cout << "Undefined Instruction:\t" << std::hex << instruction << std::dec << "\n";
                        return &und;
                    }
                    else {
                        if (cpu->canPrint()) std::cout << "Single Data Transfer:\t" << std::hex << instruction << std::dec << "\n";
                        return &ldr_str;
                    }
                    break;
                }
                case 0b10: {
                    u32 bits543210 = (instruction >> 20) & 0b111111;
                    u32 bits7654 = (instruction >> 4) & 0b1111;

                    if ((bits543210 & 0b100000) == 0b100000) {
                        if (cpu->canPrint()) std::cout << "Branch and Link:\t" << std::hex << instruction << std::dec << "\n";
                        return &bl;
                    }
                    else {
                        if (cpu->canPrint()) std::cout << "Block Data Transfer:\t" << std::hex << instruction << std::dec << "\n";
                        cpu->PRINTSTATE();
                        return &ldm_stm<false>;
                    }
                    break;
                }
                case 0b11: {
                    u32 bits543210 = (instruction >> 20) & 0b111111;
                    u32 bits7654 = (instruction >> 4) & 0b1111;

                    if ((bits543210 & 0b110000) == 0b110000) {
                        if (cpu->canPrint() || true) std::cout << "SWI; r0 is: " << std::hex << cpu->reg[12] << std::dec << "\n";
                        cpu->PRINTSTATE();
                        return &swi;
                    }
                    std::cout << "UNIMPLEMENTED GROUP 11 INSTRUCTION:\t" << std::hex << instruction << std::hex << "\n";
                    cpu->PRINTSTATE();
                    return &DEBUG_noop;
                    break;
                }
                default: {
                    lilds__unreachable();
                }
            }
        }
    }

    namespace Thumb {
        ThumbInstruction decode(State* cpu, u16 instruction) {
            return nullptr;
        }
    }
}