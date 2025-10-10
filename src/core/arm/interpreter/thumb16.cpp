#include "../../../math.h"
#include "../../../helpers.h"
#include "../arm.h"
#include "../memory.inl"
#include "interpreter.h"
#include "alu.inl"
#include "mul.inl"

namespace Interpreter {

    namespace Thumb {

        // register operation instructions
        void moveShiftedRegister(State* cpu, u16 instruction) {
            cpu->cycles++;
            int shifttype = (instruction >> 11) & 0b11;
            u32 off = (instruction >> 6) & 0b11111;
            int rs = (instruction >> 3) & 0b111;
            int rd = (instruction >> 0) & 0b111;

            if (shifttype == 0b11) {
                printf("Thumb moveShiftedRegister shifttype = 11. HOW\n");
                lilds__crash();
            }

            cpu->reg[rd] = aluRegisterBarrelShifter<true>(cpu, shifttype, cpu->reg[rs], off, true);
            aluSetLogicFlags<>(cpu, cpu->reg[rd], rd, true);
            cpu->finishInstruction();
        }

        void addSubtract(State* cpu, u16 instruction) {
            cpu->cycles++;
            int op = (instruction >> 9) & 0b11;
            u32 rn = (instruction >> 6) & 0b111;
            u32 rs = (instruction >> 3) & 0b111;
            u32 rd = instruction & 0b111;
            [[assume(rd <= 7)]];

            switch (op) {
                case 0: { // AddS register
                    cpu->reg[rd] = aluAdd<>(cpu, cpu->reg[rs], cpu->reg[rn], rd, true);
                    break;
                }
                case 1: { // SubS register
                    cpu->reg[rd] = aluSub<>(cpu, cpu->reg[rs], cpu->reg[rn], rd, true);
                    break;
                }
                case 2: { // AddS immediate
                    cpu->reg[rd] = aluAdd<>(cpu, cpu->reg[rs], rn, rd, true);
                    break;
                }
                case 3: { // SubS immediate
                    cpu->reg[rd] = aluSub<>(cpu, cpu->reg[rs], rn, rd, true);
                    break;
                }
            }

            cpu->finishInstruction();
        }

        void moveCompareAddSubtractImmediate(State* cpu, u16 instruction) {
            cpu->cycles++;
            int op = (instruction >> 11) & 0b11;
            u32 rd = (instruction << 8) & 0b111;
            u32 nn = instruction & 0xff;

            switch (op) {
                case 0: { // MovS
                    cpu->reg[rd] = nn;
                    aluSetLogicFlags<>(cpu, nn, rd, true);
                    break;
                }
                case 1: { // Cmp
                    (void)aluSub<true>(cpu, cpu->reg[rd], nn, rd, true);
                    break;
                }
                case 2: { // AddS
                    cpu->reg[rd] = aluAdd<>(cpu, cpu->reg[rd], nn, rd, true);
                    break;
                }
                case 3: { // SubS
                    cpu->reg[rd] = aluSub<>(cpu, cpu->reg[rd], nn, rd, true);
                    break;
                }
            }

            cpu->finishInstruction();
        }

        void aluOperations(State* cpu, u16 instruction) {
            cpu->cycles++;
            if (cpu->exeStage == 0) {
                int op = (instruction >> 6) & 0b1111;
                u32 rs = (instruction >> 3) & 0b111;
                u32 rd = instruction & 0b111;
                [[assume(rd <= 7)]];

                switch (op) {
                    case 0x0: { // ANDS
                        cpu->reg[rd] = cpu->reg[rd] & cpu->reg[rs];
                        aluSetLogicFlags<>(cpu, cpu->reg[rd], rd, true);
                        cpu->finishInstruction();
                        break;
                    }
                    case 0x1: { // EORS
                        cpu->reg[rd] = cpu->reg[rd] ^ cpu->reg[rs];
                        aluSetLogicFlags<>(cpu, cpu->reg[rd], rd, true);
                        cpu->finishInstruction();
                        break;
                    }
                    case 0x2: { // LSLS
                        cpu->reg[rd] = aluRegisterBarrelShifter<true>(cpu, 0, cpu->reg[rd], cpu->reg[rs] & 0xff, true);
                        aluSetLogicFlags<>(cpu, cpu->reg[rd], rd, true);
                        cpu->tmp[0] = 1;
                        break;
                    }
                    case 0x3: { // LSRS
                        cpu->reg[rd] = aluRegisterBarrelShifter<true>(cpu, 1, cpu->reg[rd], cpu->reg[rs] & 0xff, true);
                        aluSetLogicFlags<>(cpu, cpu->reg[rd], rd, true);
                        cpu->tmp[0] = 1;
                        break;
                    }
                    case 0x4: { // ASRS
                        cpu->reg[rd] = aluRegisterBarrelShifter<true>(cpu, 2, cpu->reg[rd], cpu->reg[rs] & 0xff, true);
                        aluSetLogicFlags<>(cpu, cpu->reg[rd], rd, true);
                        cpu->tmp[0] = 1;
                        break;
                    }
                    case 0x5: { // ADCS
                        cpu->reg[rd] = aluAddCarry<>(cpu, cpu->reg[rd], cpu->reg[rs], rd, true);
                        cpu->finishInstruction();
                        break;
                    }
                    case 0x6: { // SBCS
                        cpu->reg[rd] = aluSubCarry<>(cpu, cpu->reg[rd], cpu->reg[rs], rd, true);
                        cpu->finishInstruction();
                        break;
                    }
                    case 0x7: { // RORS
                        cpu->reg[rd] = aluRegisterBarrelShifter<true>(cpu, 3, cpu->reg[rd], cpu->reg[rs] & 0xff, true);
                        aluSetLogicFlags<>(cpu, cpu->reg[rd], rd, true);
                        cpu->tmp[0] = 1;
                        break;
                    }
                    case 0x8: { // TST
                        aluSetLogicFlags<true>(cpu, cpu->reg[rd] & cpu->reg[rs], rd, true);
                        cpu->finishInstruction();
                        break;
                    }
                    case 0x9: { // NEGS
                        cpu->reg[rd] = aluSub<>(cpu, 0, cpu->reg[rd], rd, true);
                        cpu->finishInstruction();
                        break;
                    }
                    case 0xA: { // CMP
                        (void)aluSub<true>(cpu, cpu->reg[rd], cpu->reg[rs], rd, true);
                        cpu->finishInstruction();
                        break;
                    }
                    case 0xB: { // CMN
                        (void)aluAdd<true>(cpu, cpu->reg[rd], cpu->reg[rs], rd, true);
                        cpu->finishInstruction();
                        break;
                    }
                    case 0xC: { // ORRS
                        cpu->reg[rd] = cpu->reg[rd] | cpu->reg[rs];
                        aluSetLogicFlags<>(cpu, cpu->reg[rd], rd, true);
                        cpu->finishInstruction();
                        break;
                    }
                    case 0xD: { // MULS
                        u32 res = (u64)cpu->reg[rd] * (u64)cpu->reg[rs];
                        cpu->writeReg(rd, res);
                        mul32SetNZCFlags(cpu, res);
                        if (cpu->type == Type::Arm7)
                            cpu->tmp[0] = mulGetICycles(cpu->reg[rs], false);
                        else
                            cpu->tmp[0] = 3; // "ARMv5 (m=3; fucking slow, no matter of MSBs of Rd value)"
                        break;
                    }
                    case 0xE: { // ORRS
                        cpu->reg[rd] = cpu->reg[rd] & (~cpu->reg[rs]);
                        aluSetLogicFlags<>(cpu, cpu->reg[rd], rd, true);
                        cpu->finishInstruction();
                        break;
                    }
                    case 0xF: { // MVNS
                        cpu->reg[rd] = ~cpu->reg[rs];
                        aluSetLogicFlags<>(cpu, cpu->reg[rd], rd, true);
                        cpu->finishInstruction();
                        break;
                    }

                }
            }
            else if (cpu->exeStage == cpu->tmp[0]) {
                cpu->finishInstruction();
            }
        }

        void hiRegisterOperations(State* cpu, u16 instruction) {
            cpu->cycles++;
            int op = (instruction >> 8) & 0b11;
            u32 rs = (instruction >> 3) & 0b1111;
            u32 rd_3 = (instruction >> 7) & 1;
            u32 rd = (instruction & 0b111) | (rd_3 << 3);

            switch (op) {
                case 0: { // ADD
                    cpu->reg[rd] = aluAdd<>(cpu, cpu->reg[rd], cpu->reg[rs] + 0*(rs == 15), rd, false);
                    break;
                }
                case 1: { // CMP
                    (void)aluSub<true>(cpu, cpu->reg[rd], cpu->reg[rs] + 0*(rs == 15), rd, true);
                    break;
                }
                case 2: { // MOV
                    cpu->writeReg(rd, cpu->reg[rs] + 0*(rs == 15));
                    break;
                }
                case 3: { // BX / BLX
                    cpu->cpsr.t = (bool)(cpu->reg[rs] & 1);
                    u32 addr = cpu->reg[rs] + (rs == 15) * 0;
                    if (rd_3)
                        cpu->reg[14] = cpu->reg[15] - 2 + 1; // Thumb bit set
                    cpu->reg[15] = addr;
                    cpu->issuePipelineFlush();
                    break;
                }
            }

            cpu->finishInstruction();
        }

        // memory load / store instructions 🫩
        void loadPCRelative(State* cpu, u16 instruction) {
            cpu->cycles++;
            if (cpu->exeStage == 0) {
                u32 rd = (instruction >> 8) & 0b111;
                u32 nn = instruction & 0xff;

                u32 addr = (cpu->reg[15] + 0 + nn*4);
                cpu->reg[rd] = cpu->read32(addr & 0xffff'fffc, Access::N);
            }
            else {
                cpu->finishInstruction();
            }
        }

        void loadStoreWithRegisterOffset(State* cpu, u16 instruction) {
            cpu->cycles++;
            if (cpu->exeStage == 0) {
                int op = (instruction >> 10) & 0b11;
                u32 ro = (instruction >> 6) & 0b111;
                u32 rb = (instruction >> 3) & 0b111;
                u32 rd = instruction & 0b111;

                u32 addr = cpu->reg[rb] + cpu->reg[ro];
                switch (op) {
                    case 0: { // STR
                        cpu->write32(addr & 0xffff'fffc, cpu->reg[rd], Access::N);
                        cpu->finishInstruction();
                        if (cpu->type == Type::Arm7)
                            cpu->nextInstructionAccessType = Access::N; // I'm assuming this is the same as the arm variant
                        break;
                    }
                    case 1: { // STRB
                        cpu->write8(addr, (u8)cpu->reg[rd], Access::N);
                        cpu->finishInstruction();
                        if (cpu->type == Type::Arm7)
                            cpu->nextInstructionAccessType = Access::N;
                        break;
                    }
                    case 2: { // LDR
                        cpu->reg[rd] = bitRotateRight(cpu->read32(addr & 0xffff'fffc, Access::N), 32, (addr & 3)*8);
                        break;
                    }
                    case 3: { // LDRB
                        cpu->reg[rd] = cpu->read8(addr, Access::N);
                        break;
                    }
                }
            }
            else {
                cpu->finishInstruction();
            }
        }

        void loadStoreSignExtendedByteHalfword(State* cpu, u16 instruction) {
            cpu->cycles++;
            if (cpu->exeStage == 0) {
                int op = (instruction >> 10) & 0b11;
                u32 ro = (instruction >> 6) & 0b111;
                u32 rb = (instruction >> 3) & 0b111;
                u32 rd = instruction & 0b111;

                u32 addr = cpu->reg[rb] + cpu->reg[ro];
                switch (op) {
                    case 0: { // STRH
                        cpu->write16(addr & 0xffff'fffe, cpu->reg[rd], Access::N);
                        cpu->finishInstruction();
                        if (cpu->type == Type::Arm7)
                            cpu->nextInstructionAccessType = Access::N;
                        break;
                    }
                    case 1: { // LDRSB
                        u32 val = cpu->read8(addr, Access::N);
                        cpu->reg[rd] = val | 0xffff'ff00 * (val >> 7);
                        break;
                    }
                    case 2: { // LDRH
                        if (cpu->type == Type::Arm7)
                            cpu->reg[rd] = bitRotateRight(cpu->read16(addr & 0xffff'fffe, Access::N), 32, (addr & 1) * 8); // Halfword address forcibly aligned, read rotated right by steps of 8 when not aligned
                        else
                            cpu->reg[rd] = cpu->read16(addr & 0xffff'fffe, Access::N);
                        break;
                    }
                    case 3: { // LDRSH
                        if (cpu->type == Type::Arm7) {
                            // Misaligned (ARM7) LDRSH reads result in a literal LDRSB read instead
                            if (addr & 1) {
                                u32 val = cpu->read8(addr, Access::N);
                                cpu->reg[rd] = val | 0xffff'ff00 * (val >> 7);
                            }
                            else {
                                u32 val = cpu->read16(addr, Access::N);
                                cpu->reg[rd] = val | 0xffff'0000 * (val >> 15);
                            }
                        }
                        else {
                            u32 val = cpu->read16(addr & 0xffff'fffe, Access::N);
                            cpu->reg[rd] = val | 0xffff'0000 * (val >> 15);
                        }
                        break;
                    }
                }
            }
            else {
                cpu->finishInstruction();
            }
        }
        
        void loadStoreWithImmediateOffset(State* cpu, u16 instruction) {
            cpu->cycles++;
            if (cpu->exeStage == 0) {
                int op = (instruction >> 11) & 0b11;
                u32 nn = (instruction >> 6) & 0b11111;
                u32 rb = (instruction >> 3) & 0b111;
                u32 rd = instruction & 0b111;

                switch (op) {
                    case 0: { // STR
                        u32 addr = cpu->reg[rb] + 4 * nn;
                        cpu->write32(addr & 0xffff'fffc, cpu->reg[rd], Access::N);
                        cpu->finishInstruction();
                        if (cpu->type == Type::Arm7)
                            cpu->nextInstructionAccessType = Access::N; // I'm assuming this is the same as the arm variant
                        break;
                    }
                    case 1: { // LDR
                        u32 addr = cpu->reg[rb] + 4 * nn;
                        cpu->reg[rd] = bitRotateRight(cpu->read32(addr & 0xffff'fffc, Access::N), 32, (addr & 3)*8);
                        break;
                    }
                    case 2: { // STRB
                        u32 addr = cpu->reg[rb] + nn;
                        cpu->write8(addr, (u8)cpu->reg[rd], Access::N);
                        cpu->finishInstruction();
                        if (cpu->type == Type::Arm7)
                            cpu->nextInstructionAccessType = Access::N;
                        break;
                    }
                    case 3: { // LDRB
                        u32 addr = cpu->reg[rb] + nn;
                        cpu->reg[rd] = cpu->read8(addr, Access::N);
                        break;
                    }
                }
            }
            else {
                cpu->finishInstruction();
            }
        }

        void loadStoreHalfword(State* cpu, u16 instruction) {
            cpu->cycles++;
            if (cpu->exeStage == 0) {
                bool load = (instruction >> 11) & 1;
                u32 nn = (instruction >> 6) & 0b11111;
                u32 rb = (instruction >> 3) & 0b111;
                u32 rd = instruction & 0b111;

                u32 addr = cpu->reg[rb] + 2 * nn;
                if (!load) {
                    cpu->write16(addr & 0xffff'fffe, cpu->reg[rd], Access::N);
                    cpu->finishInstruction();
                    if (cpu->type == Type::Arm7)
                        cpu->nextInstructionAccessType = Access::N;
                }
                else {
                    if (cpu->type == Type::Arm7)
                        cpu->reg[rd] = bitRotateRight(cpu->read16(addr & 0xffff'fffe, Access::N), 32, (addr & 1) * 8); // Halfword address forcibly aligned, read rotated right by steps of 8 when not aligned
                    else
                        cpu->reg[rd] = cpu->read16(addr & 0xffff'fffe, Access::N);
                }
            }
            else {
                cpu->finishInstruction();
            }
        }

        void loadStoreSPRelative(State* cpu, u16 instruction) {
            cpu->cycles++;
            if (cpu->exeStage == 0) {
                bool load = (instruction >> 11) & 1;
                u32 rd = (instruction >> 8) & 0b111;
                u32 nn = instruction & 0xff;

                u32 addr = cpu->reg[13] + 4 * nn;
                if (!load) {
                    cpu->write32(addr & 0xffff'fffc, cpu->reg[rd], Access::N);
                    cpu->finishInstruction();
                    if (cpu->type == Type::Arm7)
                        cpu->nextInstructionAccessType = Access::N; // I'm assuming this is the same as the arm variant
                }
                else {
                    cpu->reg[rd] = bitRotateRight(cpu->read32(addr & 0xffff'fffc, Access::N), 32, (addr & 3)*8);
                }
            }
            else {
                cpu->finishInstruction();
            }
        }

        // memory addressing instructions (love em)
        void getRelativeAddress(State* cpu, u16 instruction) {
            cpu->cycles++;
            bool spBase = (instruction >> 11) & 1;
            u32 rd = (instruction >> 8) & 0b111;
            u32 nn = instruction & 0xff;

            cpu->reg[rd] = nn * 4;
            if (!spBase)
                cpu->reg[rd] += (cpu->reg[15] + 0) & 0xffff'fffc;
            else
                cpu->reg[rd] += cpu->reg[13];

            cpu->finishInstruction();
        }

        void addOffsetToStackPointer(State* cpu, u16 instruction) {
            cpu->cycles++;
            bool sub = (instruction >> 7) & 1;
            u32 nn = instruction & 0xff;

            if (!sub)
                cpu->reg[13] += nn * 4;
            else
                cpu->reg[13] -= nn * 4;

            cpu->finishInstruction();
        }

        // memory multiple load store instructions (TODO: actually implement these properly)
        void pushPopRegisters(State* cpu, u16 instruction) {
            bool l = (instruction >> 11) & 1;
            bool r = (instruction >> 8) & 1;
            u32 rlist = instruction & 0xff;

            bool p = !l;
            bool u = l;

            u32 inst = 0b1110'100'0'0'0'1'0'1101'0000000000000000; // LDM/STM r13!, {}
            inst |= p << 24;
            inst |= u << 23;
            inst |= l << 20;
            inst |= rlist;
            inst |= r << (14 + l);
            Arm::ldm_stm<true>(cpu, inst);
        }

        void multipleLoadStore(State* cpu, u16 instruction) {
            bool l = (instruction >> 11) & 1;
            u32 rb = (instruction >> 8) & 0b111;
            u32 rlist = instruction & 0xff;

            u32 inst = 0b1110'100'0'1'0'1'0'0000'0000000000000000; // LDM/STM rb!, {rlist}
            inst |= l << 20;
            inst |= rb << 16;
            inst |= rlist;
            Arm::ldm_stm<true>(cpu, inst);
        }

        // jump and call instructions
        void conditionalBranch(State* cpu, u16 instruction) {
            cpu->cycles++;
            if (!evalConditionCode(cpu, CC((instruction >> 8) & 0xf))) {
                cpu->finishInstruction();
                return;
            }

            s32 nn = (s32)(s8)(instruction & 0xff);
            cpu->reg[15] += nn * 2;
            cpu->issuePipelineFlush();

            cpu->finishInstruction();
        }

        void unconditionalBranch(State* cpu, u16 instruction) {
            cpu->cycles++;
            u32 nn = instruction & 0x7ff;
            nn |= ~(u32)(0x7ff) * (nn >> 10);

            cpu->reg[15] += nn * 2;
            cpu->issuePipelineFlush();

            cpu->finishInstruction();
        }

        void longBranchWithLink_1(State* cpu, u16 instruction) {
            cpu->cycles++;
            u32 nn = instruction & 0x7ff;
            nn |= ~(u32)(0x7ff) * (nn >> 10);

            cpu->reg[14] = cpu->reg[15] + (nn << 12);
            cpu->finishInstruction();
        }

        void longBranchWithLink_2(State* cpu, u16 instruction) {
            cpu->cycles++;
            bool normalBl = (instruction >> 12) & 1;
            u32 nn = instruction & 0x7ff;

            u32 addr = cpu->reg[14] + (nn << 1);
            cpu->reg[14] = cpu->reg[15] - 2 + 1; // Thumb bit set
            cpu->reg[15] = addr;
            if (!normalBl) {
                if (cpu->type != Type::Arm9) {
                    lilds__crash();
                }
                cpu->cpsr.t = false;
            }

            cpu->issuePipelineFlush();
            cpu->finishInstruction();
        }

        void softwareInterrupt(State* cpu, u16 instruction) {
            cpu->cycles++;
            // TODO
            cpu->finishInstruction();
        }

        // undefined
        void undefined(State* cpu, u16 instruction) {
            cpu->cycles++;
            cpu->finishInstruction();

            std::cout << "thumb undefined reached at pc: " << std::hex << cpu->reg[15] - 4 << std::dec << "\n";
            cpu->PRINTSTATE();
        }

        // debug
        void DEBUG_noop(State* cpu, u16 instruction) {}

    }

}