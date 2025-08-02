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
                        if (cpu->canPrint()) printf("Mul at %X: %X \n", cpu->reg[15]-8, instruction);
                        return &mul;
                    }
                    if ((bits543210 & 0b111000) == 0b001000 && bits7654 == 0b1001) {// Needs to be implemented
                        if (cpu->canPrint()) printf("Mull at %X: %X \n", cpu->reg[15]-8, instruction);
                        return &mull;
                    }
                    if ((bits543210 & 0b111011) == 0b010000 && bits7654 == 0b1001) { // ^^^
                        if (cpu->canPrint()) printf("Swp at %X: %X \n", cpu->reg[15]-8, instruction);
                        return &swp;
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
                    if ((bits543210 & 0b111111) == 0b010010 && bits7654 == 0b0001) {
                        if (cpu->canPrint()) printf("Bx at %X: %X \n", cpu->reg[15]-8, instruction);
                        return &bx<false>;
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
                        cpu->PRINTSTATE();
                        return &ldm_stm<false>;
                    }
                    break;
                }
                case 0b11: {
                    u32 bits543210 = (instruction >> 20) & 0b111111;
                    u32 bits7654 = (instruction >> 4) & 0b1111;

                    if ((bits543210 & 0b110000) == 0b110000) {
                        printf("Swi (r0=%X) at %X: %X \n", cpu->reg[0], cpu->reg[15]-8, instruction);
                        cpu->PRINTSTATE();
                        return &swi;
                    }
                    printf("Unimplemented g11 instruction at %X: %X \n", cpu->reg[15]-8, instruction);
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
            return nullptr;
        }
    }
}