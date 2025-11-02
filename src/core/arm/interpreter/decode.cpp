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
                    if ((bits543210 & 0b111100) == 0b000000 && bits7654 == 0b1001) {
                        if (cpu->canPrint()) printf("Mul at %X: %X \n", cpu->reg[15]-8, instruction);
                        return &mul;
                    }
                    if ((bits543210 & 0b111000) == 0b001000 && bits7654 == 0b1001) {
                        if (cpu->canPrint()) printf("Mull at %X: %X \n", cpu->reg[15]-8, instruction);
                        return &mull;
                    }
                    if ((bits543210 & 0b111011) == 0b010000 && bits7654 == 0b1001) {
                        if (cpu->canPrint()) printf("Swp at %X: %X \n", cpu->reg[15]-8, instruction);
                        return &swp;
                    }
                    if (cpu->type == Type::Arm9 && (bits543210 & 0b111001) == 0b010000 && (bits7654 & 0b1001) == 0b1000) {
                        if (cpu->canPrint()) printf("Smul at %X: %X \n", cpu->reg[15]-8, instruction);
                        return &smul;
                    }
                    if ((bits543210 & 0b100000) == 0b000000 && (bits7654 & 0b1001) == 0b1001) {
                        if (cpu->canPrint()) printf("Ldrh_strh at %X: %X \n", cpu->reg[15]-8, instruction);
                        return &ldrh_strh;
                    }
                    if ((bits543210 & 0b111011) == 0b010000 && bits7654 == 0b0000) {
                        if (cpu->canPrint()) printf("Mrs at %X: %X \n", cpu->reg[15]-8, instruction);
                        return &mrs;
                    }
                    if (
                        ((bits543210 & 0b111011) == 0b010010 && bits7654 == 0b0000) ||  // Register ; TODO: optimize this fuckass thing here
                        ((bits543210 & 0b111011) == 0b110010)                           // Immediate
                    ) {
                        if (cpu->canPrint()) printf("Msr at %X: %X \n", cpu->reg[15]-8, instruction);
                        return &msr<false>;
                    }
                    if ((bits543210 & 0b111111) == 0b010010) {
                        if (bits7654 == 0b0001) {
                            if (cpu->canPrint()) printf("Bx at %X: %X \n", cpu->reg[15]-8, instruction);
                            return &bx<false>;
                        }
                        if (cpu->type == Type::Arm9 && bits7654 == 0b0011) {
                            if (cpu->canPrint()) printf("Blx_reg at %X: %X \n", cpu->reg[15]-8, instruction);
                            return &blx_reg<false>;
                        }
                    }
                    if (cpu->type == Type::Arm9 && bits543210 == 0b010110 && bits7654 == 0b0001) {
                        if (cpu->canPrint()) printf("Clz at %X: %X \n", cpu->reg[15]-8, instruction);
                        return &clz;
                    }
                    if (cpu->type == Type::Arm9 && (bits543210 & 0b111001) == 0b010000 && bits7654 == 0b0101) {
                        if (cpu->canPrint()) printf("Qadd at %X: %X \n", cpu->reg[15]-8, instruction);
                        return &qadd;
                    }
                    if ((bits543210 & 0b111011) == 0b110000) {
                        if (cpu->canPrint()) printf("Invalid g00 instruction at %X: %X \n", cpu->reg[15]-8, instruction);
                        cpu->PRINTSTATE();
                    }
                    if (cpu->canPrint()) printf("Alu at %X: %X \n", cpu->reg[15]-8, instruction);
                    return &alu<false>;
                    break;
                }
                case 0b01: {
                    u32 bits543210 = (instruction >> 20) & 0b111111;
                    u32 bits7654 = (instruction >> 4) & 0b1111;

                    if ((bits543210 & 0b100000) == 0b100000 && (bits7654 & 1) == 1) {
                        if (cpu->canPrint()) printf("Und at %X: %X \n", cpu->reg[15]-8, instruction);
                        return &und;
                    }
                    else {
                        if (cpu->canPrint()) printf("Ldr_str at %X: %X \n", cpu->reg[15]-8, instruction);
                        return &ldr_str;
                    }
                    break;
                }
                case 0b10: {
                    u32 bits543210 = (instruction >> 20) & 0b111111;
                    u32 bits7654 = (instruction >> 4) & 0b1111;

                    if ((bits543210 & 0b100000) == 0b100000) {
                        if (cpu->canPrint()) printf("Bl at %X: %X \n", cpu->reg[15]-8, instruction);
                        return &bl;
                    }
                    else {
                        if (cpu->canPrint()) printf("Ldm_stm at %X: %X \n", cpu->reg[15]-8, instruction);
                        return &ldm_stm<false>;
                    }
                    break;
                }
                case 0b11: {
                    u32 bits543210 = (instruction >> 20) & 0b111111;
                    u32 bits7654 = (instruction >> 4) & 0b1111;

                    if ((bits543210 & 0b110000) == 0b110000) {
                        printf("Swi (r0=%X) at %X: %X \n", cpu->reg[0], cpu->reg[15]-8, instruction);
                        return &swi;
                    }
                    printf("%s Unimplemented g11 instruction at %X: %X \n", cpu->getTypeString().c_str(), cpu->reg[15]-8, instruction);
                    // cpu->PRINTSTATE();
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
            u32 bits5432109876 = instruction >> 6;

            if ((bits5432109876 & 0b1111100000) == 0b0001100000) {
                if (cpu->canPrint()) std::cout << "Thumb16_AddSubtract:\t" << std::hex << instruction << std::dec << "\n";
                return &addSubtract;
            }
            if ((bits5432109876 & 0b1110000000) == 0b0000000000) {
                if (cpu->canPrint()) std::cout << "Thumb16_MoveShiftedRegister:\t" << std::hex << instruction << std::dec << "\n";
                return &moveShiftedRegister;
            }
            if ((bits5432109876 & 0b1110000000) == 0b0010000000) {
                if (cpu->canPrint()) std::cout << "Thumb16_MovCmpAddSubImmediate:\t" << std::hex << instruction << std::dec << "\n";
                return &moveCompareAddSubtractImmediate;
            }
            if ((bits5432109876 & 0b1111110000) == 0b0100000000) {
                if (cpu->canPrint()) std::cout << "Thumb16_ALUOperations:\t" << std::hex << instruction << std::dec << "\n";
                return &aluOperations;
            }
            if ((bits5432109876 & 0b1111110000) == 0b0100010000) {
                if (cpu->canPrint()) std::cout << "Thumb16_HiRegisterOperations:\t" << std::hex << instruction << std::dec << "\n";
                return &hiRegisterOperations;
            }
            if ((bits5432109876 & 0b1111100000) == 0b0100100000) {
                if (cpu->canPrint()) std::cout << "Thumb16_PCRelativeLoad:\t" << std::hex << instruction << std::dec << "\n";
                return &loadPCRelative;
            }
            if ((bits5432109876 & 0b1111001000) == 0b0101000000) {
                if (cpu->canPrint()) std::cout << "Thumb16_LoadStoreWithRegisterOffset:\t" << std::hex << instruction << std::dec << "\n";
                return &loadStoreWithRegisterOffset;
            }
            if ((bits5432109876 & 0b1111001000) == 0b0101001000) {
                if (cpu->canPrint()) std::cout << "Thumb16_LoadStoreSignExtendedByteHalfword:\t" << std::hex << instruction << std::dec << "\n";
                return &loadStoreSignExtendedByteHalfword;
            }
            if ((bits5432109876 & 0b1110000000) == 0b0110000000) {
                if (cpu->canPrint()) std::cout << "Thumb16_LoadStoreWithImmediateOffset:\t" << std::hex << instruction << std::dec << "\n";
                return &loadStoreWithImmediateOffset;
            }
            if ((bits5432109876 & 0b1111000000) == 0b1000000000) {
                if (cpu->canPrint()) std::cout << "Thumb16_LoadStoreHalfword:\t" << std::hex << instruction << std::dec << "\n";
                return &loadStoreHalfword;
            }
            if ((bits5432109876 & 0b1111000000) == 0b1001000000) {
                if (cpu->canPrint()) std::cout << "Thumb16_SPRelativeLoadStore:\t" << std::hex << instruction << std::dec << "\n";
                return &loadStoreSPRelative;
            }
            if ((bits5432109876 & 0b1111000000) == 0b1010000000) {
                if (cpu->canPrint()) std::cout << "getRelativeAddress:\t" << std::hex << instruction << std::dec << "\n";
                return &getRelativeAddress;
            }
            if ((bits5432109876 & 0b1111111100) == 0b1011000000) {
                if (cpu->canPrint()) std::cout << "Thumb16_AddOffsetToStackPointer:\t" << std::hex << instruction << std::dec << "\n";
                return &addOffsetToStackPointer;
            }
            if ((bits5432109876 & 0b1111011000) == 0b1011010000) {
                if (cpu->canPrint()) std::cout << "Thumb16_PushPopRegisters:\t" << std::hex << instruction << std::dec << "\n";
                return &pushPopRegisters;
            }
            if ((bits5432109876 & 0b1111000000) == 0b1100000000) {
                if (cpu->canPrint()) std::cout << "Thumb16_MultipleLoadStore:\t" << std::hex << instruction << std::dec << "\n";
                return &multipleLoadStore;
            }
            if ((bits5432109876 & 0b1111111100) == 0b1101111100) {
                if (cpu->canPrint()) std::cout << "Thumb16_SoftwareInterrupt:\t" << std::hex << instruction << std::dec << "\n";
                return &softwareInterrupt;
            }
            if ((bits5432109876 & 0b1111111100) == 0b1101111000) {
                std::cout << "UNDEFINED THUMB INS:\t" << std::hex << instruction << std::dec << "\n";
                cpu->PRINTSTATE();
                return &undefined;
            }
            if ((bits5432109876 & 0b1111000000) == 0b1101000000) {
                if (cpu->canPrint()) std::cout << "Thumb16_ConditionalBranch:\t" << std::hex << instruction << std::dec << "\n";
                return &conditionalBranch;
            }
            if ((bits5432109876 & 0b1111100000) == 0b1110000000) {
                if (cpu->canPrint()) std::cout << "Thumb16_UnconditionalBranch:\t" << std::hex << instruction << std::dec << "\n";
                return &unconditionalBranch;
            }
            if ((bits5432109876 & 0b1111100000) == 0b1111000000) {
                if (cpu->canPrint()) std::cout << "Thumb16_LongBranchWithLink_1:\t" << std::hex << instruction << std::dec << "\n";
                return &longBranchWithLink_1;
            }
            if ((bits5432109876 & 0b1110100000) == 0b1110100000) {
                if (cpu->canPrint()) std::cout << "Thumb16_LongBranchWithLink_2:\t" << std::hex << instruction << std::dec << "\n";
                return &longBranchWithLink_2;
            }

            std::cout << cpu->getTypeString() << " Unimplemented THUMB instruction:\t" << std::hex << instruction << " PC:\t" << cpu->reg[15] - 4 << std::dec << "\n";
            cpu->PRINTSTATE();
            return &DEBUG_noop;
        }
    }
}