#include "../../../math.h"
#include "../../../math.cpp"
#include "../../../helpers.h"
#include "interpreter.h"

namespace Interpreter {
    using namespace Arm;

    inline void aluSetNZFlags(State* cpu, u32 res) {
        cpu->cpsr.n = (res >> 31) == 1;
        cpu->cpsr.z = res == 0;
    }
    template <bool rdIsUnused = false>
    inline void aluSetRD15Flags(State* cpu, bool s) {
        if constexpr (rdIsUnused) {
            // What happens here again?
            if (s) {
                printf("unused RD=15, S=1\n");
            }
            else {
                printf("unused RD=15, S=0\n");
            }
            lilds__crash();
        }
        else {
            if (s)
                cpu->copySPSRToCPSR();
            cpu->issuePipelineFlush();
        }
    }
    template <bool rdIsUnused = false>
    inline void aluSetLogicFlags(State* cpu, u32 res, int rd, bool s) {
        if (rd == 15) {
            aluSetRD15Flags<rdIsUnused>(cpu, s);
        }
        else if (s) {
            aluSetNZFlags(cpu, res);
        }
    }
    template <bool rdIsUnused = false>
    inline u32 aluAdd(State* cpu, u32 a, u32 b, int rd, bool s) {
        u32 res = a + b;

        if (rd == 15) {
            aluSetRD15Flags<rdIsUnused>(cpu, s);
        }
        else if (s) {
            aluSetNZFlags(cpu, res);
            cpu->cpsr.c = u32(res) < a;
            cpu->cpsr.v = (~(a ^ b) & (a ^ u32(res))) >> 31;
        }

        return res;
    }
    template <bool rdIsUnused = false>
    inline u32 aluAddCarry(State* cpu, u32 a, u32 b, int rd, bool s) {
        u32 res = a + b + cpu->cpsr.c;

        if (rd == 15) {
            aluSetRD15Flags<rdIsUnused>(cpu, s);
        }
        else if (s) {
            aluSetNZFlags(cpu, res);
            cpu->cpsr.c = u32(res) < a;
            cpu->cpsr.v = (~(a ^ b) & (a ^ u32(res))) >> 31;
        }

        return res;
    }
    template <bool rdIsUnused = false>
    inline u32 aluSub(State* cpu, u32 a, u32 b, int rd, bool s) {
        u32 res = a - b;

        if (rd == 15) {
            aluSetRD15Flags<rdIsUnused>(cpu, s);
        }
        else if (s) {
            aluSetNZFlags(cpu, res);
            cpu->cpsr.c = a >= b;
            cpu->cpsr.v = ((a ^ b) & (a ^ u32(res))) >> 31;
        }

        return res;
    }
    template <bool rdIsUnused = false>
    inline u32 aluSubCarry(State* cpu, u32 a, u32 b, int rd, bool s) {
        u32 res = a - b + cpu->cpsr.c - 1;

        if (rd == 15) {
            aluSetRD15Flags<rdIsUnused>(cpu, s);
        }
        else if (s) {
            aluSetNZFlags(cpu, res);
            cpu->cpsr.c = (u64)a >= (u64)b - (u64)cpu->cpsr.c + 1;
            cpu->cpsr.v = ((a ^ b) & (a ^ u32(res))) >> 31;
        }

        return res;
    }

    template <bool affectC>
    inline u32 aluRegisterBarrelShifter(State* cpu, int shifttype, u32 val, u32 shift, bool s) {
        switch (shifttype) {
            case 0b00: { // LSL
                if constexpr (affectC) {
                    if (s && shift != 0)
                        cpu->cpsr.c = 1 & bitShiftRight(val, 32, 32 - shift); // Should work for >= 32
                }
                return bitShiftLeft(val, 32, shift); // Logical left
            }
            case 0b01: { // LSR
                if (shift == 0)
                    shift = 32;
                if constexpr (affectC) {
                    if (s)
                        cpu->cpsr.c = 1 & bitShiftRight(val, 32, shift - 1); // Should work for >= 32
                }
                return bitShiftRight(val, 32, shift); // Logical right
            }
            case 0b10: { // ASR
                if (shift == 0 || shift > 32)
                    shift = 32;
                if constexpr (affectC) {
                    if (s)
                        cpu->cpsr.c = 1 & bitShiftRight(val, 32, shift - 1);
                }
                return bitSignedShiftRight(val, 32, shift); // Arithmetic (signed) right.
            }
            case 0b11: { // ROR
                // RRX
                if (shift == 0) {
                    int oldC = cpu->cpsr.c;
                    if constexpr (affectC) {
                        if (s)
                            cpu->cpsr.c = val & 1;
                    }
                    return (val >> 1) | (oldC << 31);
                }
                // Normal ROR
                else {
                    shift &= 0b11111;
                    if constexpr (affectC) {
                        if (s)
                            cpu->cpsr.c = 1 & (val >> (0b11111 & u32(shift - 1)));
                    }  
                    return bitRotateRight(val, 32, shift);// Rotate right
                }
            }
            default: {
                lilds__unreachable();
            }
        }
    }

    template <bool affectC>
    inline u32 aluBarrelShifter(State* cpu, bool i, bool r, u32 op2, int r15Off, bool s) {
        // 8-bit Immediate Operand
        if (i) {
            u32 shift = (op2 >> 8) & 0xf;
            u32 imm = op2 & 0xff; // 8 bit immediate zero extended to 32 bits

            if constexpr (affectC) {
                if (s && shift != 0)
                    cpu->cpsr.c = 1 & (imm >> (0b11111 & u32(shift*2 - 1)));
            }

            return bitRotateRight(imm, 32, shift*2); // ROR by twice the shift ammount
        }
        // Register Operand
        else {
            u32 rm = op2 & 0xf;
            u32 shift;

            // Register value shift
            if (r) {
                //assert(((op2 >> 7) & 1) == 0); // "The zero in bit 7 of an instruction with a register controlled shift is compulsory; a one in this bit will cause the instruction to be a multiply or undefined instruction."
                
                u32 rs = (op2 >> 8) & 0xf;
                u32 rsVal = cpu->reg[rs] + r15Off * (rs == 15); // RS can never be R15.
                shift = rsVal & 0xff;

                if (shift == 0)
                    return cpu->reg[rm] + r15Off * (rm == 15);
            }
            // Immediate value shift
            else {
                shift = (op2 >> 7) & 0b11111; // Shift ammount is a 5 bit immediate
            }

            u64 val = cpu->reg[rm] + r15Off * (rm == 15);
            uint shifttype = (op2 >> 5) & 0b11;
            return aluRegisterBarrelShifter<affectC>(cpu, shifttype, val, shift, s);
        }
    }

}