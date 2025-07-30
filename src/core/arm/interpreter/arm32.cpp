#include "../../../math.h"
#include "../../../helpers.h"
#include "../arm.h"
#include "../memory.h"
#include "interpreter.h"
#include "alu.cpp"

namespace Interpreter {
    using namespace Arm;
    
    namespace Arm {

        // branch instructions
        template <bool thumb>
        void bl(State* cpu, u32 instruction) {
            cpu->cycles++;
            if (cpu->exeStage == 0) {
                if (!evalConditionCode(cpu, CC((instruction >> 28) & 0xf))) {
                    cpu->finishInstruction();
                    return;
                }

                bool l = (instruction >> 24) & 1;
                if (l)
                    cpu->reg[14] = cpu->reg[15] - 4;

                u32 off = instruction & 0xff'ffff;
                off |= 0xff00'0000 * (off >> 23);
                constexpr u32 mul = thumb ? 2 : 4;
                off *= mul;

                cpu->reg[15] += off;
                cpu->issuePipelineFlush();
                cpu->finishInstruction();
            }
            else if (cpu->exeStage == 1) {
                cpu->pipelineFetch<0, Access::N>(thumb);
            }
            else if (cpu->exeStage == 2) {
                cpu->pipelineFetch<1, Access::S>(thumb);
                cpu->finishInstruction();
            }
        }

        template <bool thumb>
        void bx(State* cpu, u32 instruction) {
            cpu->cycles++;
            if (cpu->exeStage == 0) {
                if (!evalConditionCode(cpu, CC((instruction >> 28) & 0xf))) {
                    cpu->finishInstruction();
                    return;
                }

                uint rn = instruction & 0xf;
                cpu->setThumbMode((bool)(cpu->reg[rn] & 1));
                u32 addr = cpu->reg[rn];

                // if constexpr (thumb) {
                //     // bx thumb behavior
                //     if (rn == 15)
                //         addr &= ~2;
                // }
                addr & 0xffff'fffc;

                cpu->reg[15] = addr;
                cpu->issuePipelineFlush();
                cpu->finishInstruction();
            }
            else if (cpu->exeStage == 1) {
                cpu->pipelineFetch<0, Access::N>(thumb);
            }
            else if (cpu->exeStage == 2) {
                cpu->pipelineFetch<1, Access::S>(thumb);
                cpu->finishInstruction();
            }
        }

        // alu
        template <bool thumb>
        void alu(State* cpu, u32 instruction) {
            cpu->cycles++;
            if (cpu->exeStage == 0) {
                if (!evalConditionCode(cpu, CC((instruction >> 28) & 0xf))) {
                    cpu->finishInstruction();
                    return;
                }

                bool i = (instruction >> 25) & 1;
                bool r = (instruction >> 4) & 1;
                u32 operType = (instruction >> 21) & 0xf;
                bool s = (instruction >> 20) & 1;
                u32 rn = (instruction >> 16) & 0xf;
                u32 rd = (instruction >> 12) & 0xf;

                bool rnIsR15 = rn == 15;
                int r15Off; // fake emulate the memory stage
                if constexpr (thumb)
                    r15Off = 2;

                if (!i && r) {
                    if constexpr (!thumb)
                        r15Off = 4;
                }
                else {
                    cpu->finishInstruction();
                    if constexpr (!thumb)
                        r15Off = 0;
                }

                u32 op1 = cpu->reg[rn] + r15Off * rnIsR15; 
                u32 op2 = (instruction >> 0) & 0xfff;

                switch (operType) {
                    case 0x0: { // AND (logical)
                        op2 = aluBarrelShifter<true>(cpu, i, r, op2, r15Off, s);
                        u32 res = op1 & op2;
                        aluSetLogicFlags(cpu, res, rd, res);
                        cpu->reg[rd] = res;
                        break;
                    }
                    case 0x1: { // EOR (logical)
                        op2 = aluBarrelShifter<true>(cpu, i, r, op2, r15Off, s);
                        u32 res = op1 ^ op2;
                        aluSetLogicFlags(cpu, res, rd, res);
                        cpu->reg[rd] = res;
                        break;
                    }
                    case 0x2: { // SUB (arithmetic)
                        op2 = aluBarrelShifter<false>(cpu, i, r, op2, r15Off, false);
                        u32 res = aluSub(cpu, op1, op2, rd, s);
                        cpu->reg[rd] = res;
                        break;
                    }
                    case 0x3: { // RSB (arithmetic)
                        op2 = aluBarrelShifter<false>(cpu, i, r, op2, r15Off, false);
                        u32 res = aluSub(cpu, op2, op1, rd, s);
                        cpu->reg[rd] = res;
                        break;
                    }
                    case 0x4: { // ADD (arithmetic)
                        op2 = aluBarrelShifter<false>(cpu, i, r, op2, r15Off, false);
                        u32 res = aluAdd(cpu, op1, op2, rd, s);
                        cpu->reg[rd] = res;
                        break;
                    }
                    case 0x5: { // ADC (arithmetic)
                        op2 = aluBarrelShifter<false>(cpu, i, r, op2, r15Off, false);
                        u32 res = aluAddCarry(cpu, op1, op2, rd, s);
                        cpu->reg[rd] = res;
                        break;
                    }
                    case 0x6: { // SBC (arithmetic)
                        op2 = aluBarrelShifter<false>(cpu, i, r, op2, r15Off, false);
                        u32 res = aluSubCarry(cpu, op1, op2, rd, s);
                        cpu->reg[rd] = res;
                        break;
                    }
                    case 0x7: { // RSC (arithmetic)
                        op2 = aluBarrelShifter<false>(cpu, i, r, op2, r15Off, false);
                        u32 res = aluSubCarry(cpu, op2, op1, rd, s);
                        cpu->reg[rd] = res;
                        break;
                    }
                    case 0x8: { // TST (logical)
                        op2 = aluBarrelShifter<true>(cpu, i, r, op2, r15Off, s);
                        u32 res = op1 & op2;
                        aluSetLogicFlags<true>(cpu, res, rd, res);
                        break;
                    }
                    case 0x9: { // TEQ (logical)
                        op2 = aluBarrelShifter<true>(cpu, i, r, op2, r15Off, s);
                        u32 res = op1 ^ op2;
                        aluSetLogicFlags<true>(cpu, res, rd, res);
                        break;
                    }
                    case 0xA: { // CMP (arithmetic)
                        op2 = aluBarrelShifter<false>(cpu, i, r, op2, r15Off, false);
                        u32 res = aluSub<true>(cpu, op1, op2, rd, s);
                        break;
                    }
                    case 0xB: { // CMN (arithmetic)
                        op2 = aluBarrelShifter<false>(cpu, i, r, op2, r15Off, false);
                        u32 res = aluAdd<true>(cpu, op1, op2, rd, s);
                        break;
                    }
                    case 0xC: { // ORR (logical)
                        op2 = aluBarrelShifter<true>(cpu, i, r, op2, r15Off, s);
                        u32 res = op1 | op2;
                        aluSetLogicFlags(cpu, res, rd, res);
                        cpu->reg[rd] = res;
                        break;
                    }
                    case 0xD: { // MOV (logical)
                        op2 = aluBarrelShifter<true>(cpu, i, r, op2, r15Off, s);
                        aluSetLogicFlags(cpu, op2, rd, res);
                        cpu->reg[rd] = op2;
                        break;
                    }
                    case 0xE: { // BIC (logical)
                        op2 = aluBarrelShifter<true>(cpu, i, r, op2, r15Off, s);
                        u32 res = op1 & (~op2);
                        aluSetLogicFlags(cpu, res, rd, res);
                        cpu->reg[rd] = res;
                        break;
                    }
                    case 0xF: { // MVN (logical)
                        op2 = aluBarrelShifter<true>(cpu, i, r, op2, r15Off, s);
                        u32 res = ~op2;
                        aluSetLogicFlags(cpu, res, rd, res);
                        cpu->reg[rd] = res;
                        break;
                    }
                }
            }
            else {
                // If exeStage advances to 1, r (shift by register) is on
                cpu->finishInstruction();
            }
        }

        // mul
        void mul(State* cpu, u32 instruction) {
            cpu->cycles++;
            if (cpu->exeStage == 0) {
                if (!evalConditionCode(cpu, CC((instruction >> 28) & 0xf))) {
                    cpu->finishInstruction();
                    return;
                }

                bool a = (instruction >> 21) & 1;
                bool s = (instruction >> 20) & 1;
                u32 rd = (instruction >> 16) & 0xf;
                u32 rn = (instruction >> 12) & 0xf;
                u32 rs = (instruction >> 8) & 0xf;
                u32 rm = (instruction >> 0) & 0xf;

                u64 res = (u64)cpu->reg[rm] * (u64)cpu->reg[rs] + (u64)(cpu->reg[rn] * a);
                cpu->writeReg(rd, (u32)res);

                if (s) {
                    cpu->cpsr.n = (res >> 63) == 1;
                    cpu->cpsr.z = res == 0;
                    if (cpu->type == Type::Arm7)
                        cpu->cpsr.c = false; // Set by the calculation on ARMv4 only
                }

                int m = mulGetICycles(cpu->reg[rs], a);
                cpu->tmp[0] = m;
            }
            else if (cpu->exeStage == cpu->tmp[0]) {
                // tmp is set to r, where r is the number of i cycles, depending on Rs's value
                cpu->finishInstruction();
            }
        }
        void mull(State* cpu, u32 instruction) {
            cpu->cycles++;
            if (cpu->exeStage == 0) {
                if (!evalConditionCode(cpu, CC((instruction >> 28) & 0xf))) {
                    cpu->finishInstruction();
                    return;
                }

                bool signedMul = (instruction >> 22) & 1;
                bool a = (instruction >> 21) & 1;
                bool s = (instruction >> 20) & 1;
                u32 rdHi = (instruction >> 16) & 0xf;
                u32 rdLo = (instruction >> 12) & 0xf;
                u32 rs = (instruction >> 8) & 0xf;
                u32 rm = (instruction >> 0) & 0xf;

                u64 res;
                if (!signedMul) {
                    res = (u64)cpu->reg[rm] * (u64)cpu->reg[rs];
                    res += a * u64((u64(cpu->reg[rdHi]) << 32) | u64(cpu->reg[rdLo]));
                }
                else {
                    res = (s64)((s32)(cpu->reg[rm])) * (s64)((s32)(cpu->reg[rs]));
                    res += a * s64((u64(cpu->reg[rdHi]) << 32) | u64(cpu->reg[rdLo]));
                }
                cpu->reg[rdHi] = (u32)(res >> 32);
                cpu->reg[rdLo] = (u32)res;

                if (rdHi == 15 || rdLo == 15)
                    cpu->issuePipelineFlush();

                if (s) {
                    cpu->cpsr.n = (res >> 63) == 1;
                    cpu->cpsr.z = res == 0;
                    if (cpu->type == Type::Arm7)
                        cpu->cpsr.c = false; // Set by the calculation on ARMv4 only
                }

                int m = mulGetICycles(cpu->reg[rs], a) + 1;
                cpu->tmp[0] = m;
            }
            else if (cpu->exeStage == cpu->tmp[0]) {
                // tmp is set to r, where r is the number of i cycles, depending on Rs's value
                cpu->finishInstruction();
            }
        }
        void smul(State* cpu, u32 instruction) {
            cpu->cycles++;
            if (cpu->exeStage == 0) {
                if (!evalConditionCode(cpu, CC((instruction >> 28) & 0xf))) {
                    cpu->finishInstruction();
                    return;
                }

                u32 operType = (instruction >> 21) & 7;
                u32 rdHi = (instruction >> 16) & 0xf;
                u32 rdLo = (instruction >> 12) & 0xf;
                u32 rs = (instruction >> 8) & 0xf;
                u32 rm = (instruction >> 0) & 0xf;
                bool y = (instruction >> 6) & 1;
                bool x = (instruction >> 5) & 1;
                s16 halfRs = (cpu->reg[rs] >> (y*16)) & 0xffff;

                switch (operType) {
                    case 1: { // SMLAxy
                        s16 halfRm = (cpu->reg[rm] >> (x*16)) & 0xffff;

                        u32 a = (s32)halfRs * (s32)halfRm;
                        u32 res = a + cpu->reg[rdLo];
                        cpu->reg[rdHi] = res;
                        cpu->cpsr.q = (~(a ^ cpu->reg[rdLo]) & (a ^ u32(res))) >> 31;

                        if (rdHi == 15)
                            cpu->issuePipelineFlush();
                        cpu->finishInstruction();
                        break;
                    }
                    case 2: { // SMLAWy
                        u32 a = ((s32)halfRs * (s32)cpu->reg[rm]) / (s32)0x10000;
                        u32 res = a + cpu->reg[rdLo];
                        cpu->reg[rdHi] = res;
                        cpu->cpsr.q = (~(a ^ cpu->reg[rdLo]) & (a ^ u32(res))) >> 31;

                        if (rdHi == 15)
                            cpu->issuePipelineFlush();
                        cpu->finishInstruction();
                        break;
                    }
                    case 3: { // SMULWy
                        u32 res = ((s32)halfRs * (s32)cpu->reg[rm]) / (s32)0x10000;
                        cpu->reg[rdHi] = res;

                        if (rdHi == 15)
                            cpu->issuePipelineFlush();
                        cpu->finishInstruction();
                        break;
                    } 
                    case 4: { // SMLALxy
                        s16 halfRm = (cpu->reg[rm] >> (x*16)) & 0xffff;
                        s64 rdHiLo = (u64(cpu->reg[rdHi]) << 32) | u64(cpu->reg[rdLo]);

                        u64 res = (s64)halfRs * (s64)halfRm + rdHiLo;
                        cpu->reg[rdHi] = (u32)(res >> 32);
                        cpu->reg[rdLo] = (u32)res;
                        if (rdHi == 15 || rdLo == 15)
                            cpu->issuePipelineFlush();
                        break;
                    }
                    case 5: { // SMULxy
                        s16 halfRm = (cpu->reg[rm] >> (x*16)) & 0xffff;
                        u32 res = (s32)halfRs * (s32)halfRm;
                        cpu->reg[rdHi] = res;

                        if (rdHi == 15)
                            cpu->issuePipelineFlush();
                        cpu->finishInstruction();
                        break;
                    }
                    default: {
                        printf("SMUL funny operType %d", operType);
                        assert(0);
                    }
                }
            }
            else {
                // TODO: handle interlocks
                cpu->finishInstruction(); // 1 i cycle for SMLALxy
            }
        }

        void clz(State* cpu, u32 instruction) {
            cpu->cycles++;
            cpu->finishInstruction();
            if (!evalConditionCode(cpu, CC((instruction >> 28) & 0xf))) {
                return;
            }

            u32 rd = (instruction >> 12) & 0xf;
            u32 rm = instruction & 0xf;

            bool rmIsZero = cpu->reg[rm] == 0;
            cpu->writeReg(rd, !rmIsZero * __clz(cpu->reg[rm]) + rmIsZero * 32);
        }

        void qadd(State* cpu, u32 instruction) {
            cpu->cycles++;
            cpu->finishInstruction();
            if (!evalConditionCode(cpu, CC((instruction >> 28) & 0xf))) {
                return;
            }

            u32 rn = (instruction >> 16) & 0xf;
            u32 rd = (instruction >> 12) & 0xf;
            u32 rm = (instruction >> 0) & 0xf;
            bool doubleMode = (instruction >> 22) & 1;
            bool subtract = (instruction >> 21) & 1;

            s64 res;
            s64 a = (s64)(s32)cpu->reg[rm];
            s64 b = (s64)(s32)cpu->reg[rn];

            if (doubleMode) {
                if (b > 0x3fff'ffff) {
                    b = 0x7fff'ffff;
                    cpu->cpsr.q = true;
                }
                else if (b < -0x4000'0000) {
                    b = -0x4000'0000;
                    cpu->cpsr.q = true;
                }
                else {
                    b *= 2;
                }
            }

            if (subtract)
                res = a - b;
            else
                res = a + b;

            if (res > 0x7fff'ffff) {
                cpu->writeReg(rd, 0x7fff'ffff);
                cpu->cpsr.q = true;
            }
            else if (res < -0x8000'0000) {
                cpu->writeReg(rd, (u32)-0x8000'0000);
                cpu->cpsr.q = true;
            }
            else {
                cpu->writeReg(rd, (u32)res);
            }
        }

        void mrs(State* cpu, u32 instruction) {
            cpu->cycles++;
            cpu->finishInstruction();
            if (!evalConditionCode(cpu, CC((instruction >> 28) & 0xf))) {
                return;
            }

            bool fromSpsr = (instruction >> 22) & 1;
            u32 rd = (instruction >> 12) & 0xf;

            int bank = cpu->getModeBank(cpu->cpsr.mode) * fromSpsr;
            cpu->writeReg(rd, (bank == 0) ? cpu->readCPSR() : cpu->spsr[bank]);
        }

        template <bool thumbExe>
        void msr(State* cpu, u32 instruction) {
            cpu->cycles++;
            cpu->finishInstruction();
            if (!evalConditionCode(cpu, CC((instruction >> 28) & 0xf))) {
                return;
            }

            bool i = (instruction >> 25) & 1;
            bool toSpsr = (instruction >> 22) & 1;
            bool f = (instruction >> 19) & 1;
            bool c = (instruction >> 16) & 1;

            u32 writeMask = c*0x1f + f*(cpu->type == Type::Arm9 ? 0xf800'0000 : 0xf000'0000);

            u32 op = instruction & 0xfff;
            u32 r15Off = constexpr thumbExe ? 4 : 8; // TODO: not right?
            op = aluBarrelShifter<false>(cpu, i, false, op, r15Off, false); // TODO: can be optimized to not check for (not possible) register shift (I=0, R=1)

            if (toSpsr) {
                int bank = cpu->getModeBank(cpu->cpsr.mode);
                if (bank != 0)
                    cpu->spsr[bank] = (cpu->spsr[bank] & ~writeMask) | (op & writeMask);
            }
            else {
                cpu->writeCPSR((cpu->readCPSR() & ~writeMask) | (op & writeMask)); // can be optimized to remove a few arm type (inlined?)
            }
        }

        void ldr_str(State* cpu, u32 instruction) {
            cpu->cycles++;
            if (cpu->exeStage == 0) {
                if (!evalConditionCode(cpu, CC((instruction >> 28) & 0xf))) {
                    cpu->finishInstruction();
                    return;
                }

                bool i = (instruction >> 25) & 1;
                bool p = (instruction >> 24) & 1;
                bool u = (instruction >> 23) & 1;
                u32 rn = (instruction >> 16) & 0xf;
                u32 rd = (instruction >> 12) & 0xf;
                u32 off = (instruction >> 0) & 0xfff;

                if (i)
                    off = aluBarrelShifter<false>(cpu, false, false, off, 0, false);

                if (!u)
                    off = -(s32)off;

                u32 addr = cpu->reg[rn] + p * off;

                cpu->tmp[0] = addr;
                cpu->tmp[1] = off;
            }
            else if (cpu->exeStage == 1) {
                u32 addr = cpu->tmp[0];
                u32 off = cpu->tmp[1];
                // TODO: not a fan of this, what else can we do? maybe a tickOtherDSComponents func in the future
                bool l = (instruction >> 20) & 1;
                bool p = (instruction >> 24) & 1;
                bool b = (instruction >> 22) & 1;
                bool w = (instruction >> 21) & 1;
                u32 rn = (instruction >> 16) & 0xf;
                u32 rd = (instruction >> 12) & 0xf;

                if (l) {
                    if (b)
                        cpu->reg[rd] = cpu->read8(addr, Access::N);
                    else
                        cpu->reg[rd] = bitRotateRight(cpu->read32(addr & 0xffff'fffc, Access::N), 32, (addr & 3)*8);

                    if (rd == 15) {
                        cpu->issuePipelineFlush();
                        if (cpu->type == Type::Arm9)
                            cpu->setThumb(cpu->reg[15] & 1);
                    }
                }
                // Store
                else {
                    if (b)
                        cpu->write8(addr, u8(cpu->reg[rd]), Access::N); // TODO: what abt thumb, are str pc's even possible in tumb
                    else
                        cpu->write32(addr & 0xffff'fffc, cpu->reg[rd], Access::N);

                    cpu->finishInstruction();
                    if (cpu->type == Type::Arm7)
                        cpu->nextInstructionAccessType = Access::N; // 1S + 1N in ARM9, 2N in ARM7, src: gbatek
                }

                addr += !p * off;
                if ((w || p == 0) && (rd != rn || !l)) // TODO: check if the second half of this condition is correct behavior
                    cpu->writeReg(rn, addr);
            }
            else {
                cpu->finishInstruction(); // Load I cycle
            }
        }

        void ldrh_strh(State* cpu, u32 instruction) {
            cpu->cycles++;
            if (cpu->exeStage == 0) {
                if (!evalConditionCode(cpu, CC((instruction >> 28) & 0xf))) {
                    cpu->finishInstruction();
                    return;
                }

                bool p = (instruction >> 24) & 1;
                bool u = (instruction >> 23) & 1;
                bool i = (instruction >> 22) & 1;
                bool w = (instruction >> 21) & 1;
                bool l = (instruction >> 20) & 1;
                auto rn = (instruction >> 16) & 0xf;
                auto rd = (instruction >> 12) & 0xf;

                u32 off;
                if (i)
                    off = (instruction & 0xf) | (((instruction >> 8) & 0xf) << 4);
                else
                    off = cpu->reg[instruction & 0xf];
            
                if (!u)
                    off = -(s32)off;

                u32 addr = cpu->reg[rn] + p * off;

                cpu->tmp[0] = addr;
                cpu->tmp[1] = off;
            }
            else if (cpu->exeStage == 1) {
                u32 addr = cpu->tmp[0];
                u32 off = cpu->tmp[1];

                bool p = (instruction >> 24) & 1;
                bool u = (instruction >> 23) & 1;
                bool i = (instruction >> 22) & 1;
                bool w = (instruction >> 21) & 1;
                bool l = (instruction >> 20) & 1;
                auto rn = (instruction >> 16) & 0xf;
                auto rd = (instruction >> 12) & 0xf;

                // 8 (6 usable) possible opcodes -- L bit is used as the most significant bit here
                auto operType = ((instruction >> 5) & 3) | (l << 2); 
                switch (operType) {
                    // When Bit 20 L=0 (Store) (and Doubleword Load/Store):
                    case 1: { // STRH
                        cpu->write16(addr & 0xffff'fffe, cpu->reg[rd], Access::N);
                        cpu->finishInstruction();
                        if (cpu->type == Type::Arm7)
                            cpu->nextInstructionAccessType = Access::N;
                        break;
                    }
                    case 2: // LDRD (ARM9)
                        break;
                    case 3: // STRD (ARM9)
                        break;
                    // When Bit 20 L=1 (Load):
                    case 5: { // LDRH
                        if (cpu->type == Type::Arm7)
                            cpu->writeReg(rd, bitRotateRight(cpu->read16(addr & 0xffff'fffe, Access::N), 32, (addr & 1) * 8)); // Halfword address forcibly aligned, read rotated right by steps of 8 when not aligned
                        else
                            cpu->writeReg(rd, cpu->read16(addr & 0xffff'fffe, Access::N));
                        break;
                    }
                    case 6: { // LDRSB
                        u32 val = cpu->read8(addr, Access::N);
                        val |= 0xffff'ff00 * (val >> 7);
                        cpu->writeReg(rd, val);
                        break;
                    }
                    case 7: { // LDRSH
                        if (cpu->type == Type::Arm7) {
                             // Misaligned (ARM7) LDRSH reads result in a literal LDRSB read instead
                            if (addr & 1) {
                                u32 val = cpu->read8(addr, Access::N);
                                val |= 0xffff'ff00 * (val >> 7);
                                cpu->writeReg(rd, val);
                            }
                            else {
                                u32 val = cpu->read16(addr, Access::N);
                                val |= 0xffff'0000 * (val >> 15);
                                cpu->writeReg(rd, val);
                            }
                        }
                        else {
                            u32 val = cpu->read16(addr & 0xffff'fffe, Access::N);
                            val |= 0xffff'0000 * (val >> 15);
                            cpu->writeReg(rd, val);
                        }
                        break;
                    }
                    default:
                        printf("Oooooooooooops! We decoded a LDRH_STRH instead of SWP!");
                        // __unreachable();
                        break;
                }

                addr += !p * off;
                if ((w || p == 0) && (rd != rn || !l)) { // NOTE: this is the same condition as Single Data Transfer (i assume)
                    cpu->writeReg(rn, addr);
                }
            }
            else {
                cpu->finishInstruction(); // Load I cycle
            }
        }

    }
}