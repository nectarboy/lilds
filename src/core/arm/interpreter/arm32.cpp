#include "../../../math.h"
#include "../../../helpers.h"
#include "../arm.h"
#include "../memory.inl"
#include "interpreter.h"
#include "alu.inl"
#include "mul.inl"

// TODO: templatize all functions checking for arm types later

namespace Interpreter {
    using namespace Arm;
    
    namespace Arm {

        // branch instructions
        void bl(State* cpu, u32 instruction) {
            cpu->cycles++;
            if (cpu->exeStage == 0) {
                CC cc = static_cast<CC>((instruction >> 28) & 0xf);
                bool isBlx = (cpu->type == Type::Arm9) & (cc == CC::UND);
                if (!isBlx && !evalConditionCode(cpu, cc)) {
                    cpu->finishInstruction();
                    return;
                }

                bool l = (instruction >> 24) & 1;
                u32 off = instruction & 0xff'ffff;
                off |= 0xff00'0000 * (off >> 23);
                off *= 4;

                cpu->cpsr.t |= (isBlx);
                off += (isBlx & l) << 1;
                if (l || isBlx) {
                    cpu->reg[14] = cpu->reg[15] - 4;
                }
                cpu->reg[15] += off;
                cpu->issuePipelineFlush();

                cpu->finishInstruction();
            }
        }

        // TODO: seperate BLX immediate handler?

        template <bool thumb>
        void bx(State* cpu, u32 instruction) {
            cpu->cycles++;
            if (cpu->exeStage == 0) {
                if (!evalConditionCode(cpu, CC((instruction >> 28) & 0xf))) {
                    cpu->finishInstruction();
                    return;
                }

                auto rn = instruction & 0xf;
                cpu->cpsr.t = (bool)(cpu->reg[rn] & 1);
                u32 addr = cpu->reg[rn];

                addr &= 0xffff'fffc;

                cpu->reg[15] = addr;
                cpu->issuePipelineFlush();
                cpu->finishInstruction();
            }
        }
        template void bx<false>(State* cpu, u32 instruction);
        template void bx<true>(State* cpu, u32 instruction);

        template <bool thumb>
        void blx_reg(State* cpu, u32 instruction) {
            cpu->cycles++;
            if (cpu->exeStage == 0) {
                if (!evalConditionCode(cpu, CC((instruction >> 28) & 0xf))) {
                    cpu->finishInstruction();
                    return;
                }

                auto rn = instruction & 0xf;
                cpu->cpsr.t = (bool)(cpu->reg[rn] & 1);
                u32 addr = cpu->reg[rn];
                addr &= 0xffff'fffc;
                
                if constexpr (thumb)
                    cpu->reg[14] = (cpu->reg[15] - 2) | 1; // Thumb bit set
                else
                    cpu->reg[14] = cpu->reg[15] - 4;

                cpu->reg[15] = addr;
                cpu->issuePipelineFlush();
                cpu->finishInstruction();
            }
        }
        template void blx_reg<false>(State* cpu, u32 instruction);
        template void blx_reg<true>(State* cpu, u32 instruction);

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

                // fake emulate the memory stage
                int r15Off; 
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
                        aluSetLogicFlags<>(cpu, res, rd, s);
                        cpu->reg[rd] = res;
                        break;
                    }
                    case 0x1: { // EOR (logical)
                        op2 = aluBarrelShifter<true>(cpu, i, r, op2, r15Off, s);
                        u32 res = op1 ^ op2;
                        aluSetLogicFlags<>(cpu, res, rd, s);
                        cpu->reg[rd] = res;
                        break;
                    }
                    case 0x2: { // SUB (arithmetic)
                        op2 = aluBarrelShifter<false>(cpu, i, r, op2, r15Off, false);
                        u32 res = aluSub<>(cpu, op1, op2, rd, s);
                        cpu->reg[rd] = res;
                        // printf("Sub res: %x \n", res);
                        break;
                    }
                    case 0x3: { // RSB (arithmetic)
                        op2 = aluBarrelShifter<false>(cpu, i, r, op2, r15Off, false);
                        u32 res = aluSub<>(cpu, op2, op1, rd, s);
                        cpu->reg[rd] = res;
                        break;
                    }
                    case 0x4: { // ADD (arithmetic)
                        op2 = aluBarrelShifter<false>(cpu, i, r, op2, r15Off, false);
                        u32 res = aluAdd<>(cpu, op1, op2, rd, s);
                        cpu->reg[rd] = res;
                        break;
                    }
                    case 0x5: { // ADC (arithmetic)
                        op2 = aluBarrelShifter<false>(cpu, i, r, op2, r15Off, false);
                        u32 res = aluAddCarry<>(cpu, op1, op2, rd, s);
                        cpu->reg[rd] = res;
                        break;
                    }
                    case 0x6: { // SBC (arithmetic)
                        op2 = aluBarrelShifter<false>(cpu, i, r, op2, r15Off, false);
                        u32 res = aluSubCarry<>(cpu, op1, op2, rd, s);
                        cpu->reg[rd] = res;
                        break;
                    }
                    case 0x7: { // RSC (arithmetic)
                        op2 = aluBarrelShifter<false>(cpu, i, r, op2, r15Off, false);
                        u32 res = aluSubCarry<>(cpu, op2, op1, rd, s);
                        cpu->reg[rd] = res;
                        break;
                    }
                    case 0x8: { // TST (logical)
                        op2 = aluBarrelShifter<true>(cpu, i, r, op2, r15Off, s);
                        u32 res = op1 & op2;
                        aluSetLogicFlags<true>(cpu, res, rd, s);
                        break;
                    }
                    case 0x9: { // TEQ (logical)
                        op2 = aluBarrelShifter<true>(cpu, i, r, op2, r15Off, s);
                        u32 res = op1 ^ op2;
                        aluSetLogicFlags<true>(cpu, res, rd, s);
                        break;
                    }
                    case 0xA: { // CMP (arithmetic)
                        op2 = aluBarrelShifter<false>(cpu, i, r, op2, r15Off, false);
                        (void)aluSub<true>(cpu, op1, op2, rd, s);
                        break;
                    }
                    case 0xB: { // CMN (arithmetic)
                        op2 = aluBarrelShifter<false>(cpu, i, r, op2, r15Off, false);
                        (void)aluAdd<true>(cpu, op1, op2, rd, s);
                        break;
                    }
                    case 0xC: { // ORR (logical)
                        op2 = aluBarrelShifter<true>(cpu, i, r, op2, r15Off, s);
                        u32 res = op1 | op2;
                        aluSetLogicFlags<>(cpu, res, rd, s);
                        cpu->reg[rd] = res;
                        break;
                    }
                    case 0xD: { // MOV (logical)
                        op2 = aluBarrelShifter<true>(cpu, i, r, op2, r15Off, s);
                        aluSetLogicFlags<>(cpu, op2, rd, s);
                        cpu->reg[rd] = op2;
                        break;
                    }
                    case 0xE: { // BIC (logical)
                        op2 = aluBarrelShifter<true>(cpu, i, r, op2, r15Off, s);
                        u32 res = op1 & (~op2);
                        aluSetLogicFlags<>(cpu, res, rd, s);
                        cpu->reg[rd] = res;
                        break;
                    }
                    case 0xF: { // MVN (logical)
                        op2 = aluBarrelShifter<true>(cpu, i, r, op2, r15Off, s);
                        u32 res = ~op2;
                        aluSetLogicFlags<>(cpu, res, rd, s);
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
        template void alu<false>(State* cpu, u32 instruction);
        template void alu<true>(State* cpu, u32 instruction);

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

                u32 res = (u64)cpu->reg[rm] * (u64)cpu->reg[rs] + (u64)(cpu->reg[rn] * a);
                cpu->writeReg(rd, (u32)res);

                if (s)
                    mul32SetNZCFlags(cpu, res);

                int m = mulGetICycles(cpu->reg[rs], a);
                cpu->tmp[0] = m;
            }
            else if (cpu->exeStage == cpu->tmp[0]) {
                // tmp is set to r, where r is the number of i cycles, depending on [Rs]'s value
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
                cpu->writeReg(rdHi, (u32)(res >> 32));
                cpu->writeReg(rdLo, (u32)res);

                if (s)
                    mul64SetNZCFlags(cpu, res);

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
                    case 0: { // SMLAxy
                        s16 halfRm = (cpu->reg[rm] >> (x*16)) & 0xffff;

                        u32 a = (s32)halfRs * (s32)halfRm;
                        u32 res = a + cpu->reg[rdLo];
                        cpu->writeReg(rdHi, res);
                        cpu->cpsr.q = (~(a ^ cpu->reg[rdLo]) & (a ^ u32(res))) >> 31;
                        cpu->finishInstruction();
                        break;
                    }
                    case 1: { // SMLAWy, SMULWy
                        u32 a = ((s32)halfRs * (s32)cpu->reg[rm]) >> 16; // TODO: is them being s32 wrong? its a 48 bit result with bottom 16 shifted right
                        u32 res = a + !x * cpu->reg[rdLo];
                        cpu->writeReg(rdHi, res);
                        if (!x)
                            cpu->cpsr.q = (~(a ^ cpu->reg[rdLo]) & (a ^ u32(res))) >> 31;
                        cpu->finishInstruction();
                        break;
                    }
                    case 2: { // SMLALxy
                        s16 halfRm = (cpu->reg[rm] >> (x*16)) & 0xffff;
                        s64 rdHiLo = ((u64)cpu->reg[rdHi] << 32) | (u64)cpu->reg[rdLo];

                        u64 res = (s64)halfRs * (s64)halfRm + rdHiLo;
                        cpu->writeReg(rdHi, (u32)(res >> 32));
                        cpu->writeReg(rdLo, (u32)res);
                        break;
                    }
                    case 3: { // SMULxy
                        s16 halfRm = (cpu->reg[rm] >> (x*16)) & 0xffff;
                        u32 res = (s32)halfRs * (s32)halfRm;
                        cpu->writeReg(rdHi, res);
                        cpu->finishInstruction();
                        break;
                    }
                    default: {
                        printf("SMUL funny operType %d \n", operType);
                        lilds__crash();
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
            u32 rm = instruction & 0xf;
            bool doubleMode = (instruction >> 22) & 1;
            bool subtract = (instruction >> 21) & 1;

            s64 res;
            s64 a = (s64)(s32)cpu->reg[rm];
            s64 b = (s64)(s32)cpu->reg[rn];

            if (doubleMode) {
                if (b > 0x3fff'ffffLL) {
                    b = 0x7fff'ffffLL;
                    cpu->cpsr.q = true;
                }
                else if (b < -0x4000'0000LL) {
                    b = -0x8000'0000LL; // -0x4000'0000 or -0x8000'0000 ??
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

            if (res > 0x7fff'ffffLL) {
                cpu->writeReg(rd, 0x7fff'ffff);
                cpu->cpsr.q = true;
            }
            else if (res < -0x8000'0000LL) {
                cpu->writeReg(rd, -0x8000'0000);
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

        template <bool thumb>
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
            constexpr u32 r15Off = thumb ? 2 : 4; // TODO: not right?
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
        template void msr<false>(State* cpu, u32 instruction);
        template void msr<true>(State* cpu, u32 instruction);

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
                        cpu->writeReg(rd, cpu->read8(addr, Access::N));
                    else
                        cpu->writeReg(rd, bitRotateRight(cpu->read32(addr & 0xffff'fffc, Access::N), 32, (addr & 3)*8));
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
                if ((w || !p) && (rd != rn || !l)) // TODO: check if the second half of this condition is correct behavior
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
                        lilds__crash();
                        break;
                    case 3: // STRD (ARM9)
                        lilds__crash();
                        break;
                    // When Bit 20 L=1 (Load):
                    case 5: { // LDRH
                        if (cpu->type == Type::Arm7)
                            cpu->writeReg(rd, bitRotateRight(cpu->read16(addr & 0xffff'fffe, Access::N), 32, (addr & 1) * 8)); // Halfword address forcibly aligned, read rotated right by steps of 8 when not aligned
                        else
                            cpu->writeReg(rd, cpu->read16(addr & 0xffff'fffe, Access::N));

                        // TODO: isnt thumb bit supposed to be r15 bit 0 in arm9 for ALL r15 writes??? (except when disabled???)
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
                        // lilds__unreachable();
                        break;
                }

                addr += !p * off;
                if ((w || !p) && (rd != rn || !l)) { // NOTE: this is the same condition as Single Data Transfer (i assume)
                    cpu->writeReg(rn, addr);
                }
            }
            else {
                cpu->finishInstruction(); // Load I cycle
            }
        }

        // TODO: an accurate LDM STM
        // * Arm9 thumb ldm_stm behaves the same as Arm7 ldm_stm? (only writeback or all quirks?)
        template <bool thumb>
        void ldm_stm(State* cpu, u32 instruction) {
            cpu->cycles++;
            cpu->finishInstruction();

            if (!evalConditionCode(cpu, CC((instruction >> 28) & 0xf)))
                return;
            bool p = (instruction >> 24) & 1;
            bool u = (instruction >> 23) & 1;
            bool s = (instruction >> 22) & 1;
            bool w = (instruction >> 21) & 1;
            bool l = (instruction >> 20) & 1;
            u32 rn = (instruction >> 16) & 0xf;
            u32 rlist = (instruction >> 0) & 0xffff;
            u32 rcount = 0;

            // Empty rlist
            if (rlist == 0) {
                if (cpu->type == Type::Arm7)
                    rlist = 0x8000;     // "Empty Rlist: R15 loaded/stored (ARMv4 only),
                rcount = 16;            //  and Rb=Rb+/-40h (ARMv4-v5)."
            }
            // Normal rlist
            else {
                for (int i = 0; i < 16; i++) {
                    // Use the commented method if firstreg is ever used
                    //if ((rlist >> i) & 1) {
                    //  if (rcount++ == 0)
                    //      firstreg = i;
                    //}
                    rcount += (rlist >> i) & 1;
                }
                // rcount = std::popcount(rlist);
            }

            bool rnInRlist = (rlist >> rn) & 1;

            // LDM with r15 and S set
            if (s && l && (rlist >> 15)) {
                cpu->copySPSRToCPSR();
                s = false;
            }

            u32 addr = cpu->reg[rn]; // TODO: what if rn=15
            u32 oldaddr = addr;
            // LDM (rn in rlist) Writeback / LDM/STM Normal Writeback
            // "Writeback with Rb included in Rlist: Store OLD base if Rb is FIRST entry in Rlist, otherwise store NEW base (STM/ARMv4), always store OLD base (STM/ARMv5), no writeback (LDM/ARMv4), writeback if Rb is "the ONLY register, or NOT the LAST register" in Rlist (LDM/ARMv5)."
            bool earlyWriteback = (l || !rnInRlist);
            if (w && earlyWriteback) {
                cpu->writeReg(rn, u ? addr + rcount * 4 : addr - rcount * 4);
            }
            // Descending order (not really)
            if (!u) {
                addr -= rcount * 4;
                p = !p;
            }

            if (cpu->canPrint()) std::cout << "block addr:\t" << std::hex << addr << std::dec << "\n";

            // Traansfer
            for (int i = 0; i < 16; i++) {
                if (((rlist >> i) & 1) == 0)
                    continue;

                addr += 4 * p;
                if (l) {
                    if (s)
                        cpu->writeUserBankReg(i, cpu->read32(addr & 0xffff'fffc, Access::S));
                    else
                        cpu->writeReg(i, cpu->read32(addr & 0xffff'fffc, Access::S));
                }
                else {
                    if (s)
                        cpu->write32(addr & 0xffff'fffc, cpu->readUserBankReg(i), Access::S);
                    else
                        cpu->write32(addr & 0xffff'fffc, cpu->reg[i], Access::S);
                }
                addr += 4 * (!p);

                // STM (rn in rlist) Writeback
                // TODO: can easily optimize this by having this check only happen in the first loop
                if (w && !earlyWriteback) {
                    cpu->writeReg(rn, u ? oldaddr + rcount * 4 : oldaddr - rcount * 4);
                    w = false;
                }
            }
        }
        template void ldm_stm<false>(State* cpu, u32 instruction);
        template void ldm_stm<true>(State* cpu, u32 instruction);

        void swp(State* cpu, u32 instruction) {
            cpu->cycles++;
            if (cpu->exeStage == 0) {
                if (!evalConditionCode(cpu, CC((instruction >> 28) & 0xf))) {
                    cpu->finishInstruction();
                    return;
                }
            }
            else if (cpu->exeStage == 1) {
                bool b = (instruction >> 22) & 1;
                u32 rn = (instruction >> 16) & 0xf;

                u32 addr = cpu->reg[rn];
                u32 data;
                if (b)
                    data = cpu->read8(addr, Access::N);
                else
                    data = cpu->read32(addr, Access::N);

                cpu->tmp[0] = data;
            }
            else if (cpu->exeStage == 2) {
                u32 data = cpu->tmp[0];

                bool b = (instruction >> 22) & 1;
                u32 rn = (instruction >> 16) & 0xf;
                u32 rd = (instruction >> 12) & 0xf;
                u32 rm = (instruction >> 0) & 0xf;

                u32 addr = cpu->reg[rn];
                if (b)
                    cpu->write8(addr, cpu->reg[rm], Access::N);
                else
                    cpu->write32(addr, cpu->reg[rm], Access::N);

                cpu->writeReg(rd, data);
            } 
            else {
                cpu->finishInstruction(); // Final I cycle
            }
        }

        void swi(State* cpu, u32 instruction) {
            cpu->cycles++;
            // TODO
            cpu->finishInstruction();

            lilds__crash();
        } 

        void und(State* cpu, u32 instruction) {
            cpu->cycles++;
            cpu->finishInstruction();

            std::cout << "arm undefined reached at pc: " << std::hex << cpu->reg[15] - 4 << std::dec << "\n";
            cpu->PRINTSTATE();
        }

        void DEBUG_noop(State* cpu, u32 instruction) {
            cpu->cycles++;
            cpu->finishInstruction();
        }

    }
}