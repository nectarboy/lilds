#pragma once
#include "../../../helpers.h"
#include "../arm.h"

// this namespace contains functions that relate to executing and decoding arm and thumb instructions
namespace Interpreter {
    using namespace Arm;

    // condition codes
    enum class CC {
        Z_SET,
        Z_CLR,
        C_SET,
        C_CLR,
        N_SET,
        N_CLR,
        V_SET,
        V_CLR,
        C_SET_AND_Z_CLR,
        C_CLR_OR_Z_SET,
        N_EQ_V,
        N_NEQ_V,
        Z_CLR_AND_N_EQ_V,
        Z_SET_OR_N_NEQ_V,
        AL,
        UND
    };
    bool evalConditionCode(State* cpu, CC cc);

    // alu functions
    inline void aluSetNZFlags(State* cpu, u32 res);
    inline void aluSetRD15Flags(State* cpu, bool s);
    inline void aluSetLogicFlags(State* cpu, u32 res, int rd, bool s);
    inline u32 aluAdd(State* cpu, u32 a, u32 b, int rd, bool s);
    inline u32 aluAddCarry(State* cpu, u32 a, u32 b, int rd, bool s);
    inline u32 aluSub(State* cpu, u32 a, u32 b, int rd, bool s);
    inline u32 aluSubCarry(State* cpu, u32 a, u32 b, int rd, bool s);
    inline u32 aluRegisterBarrelShifter(State* cpu, int shifttype, u32 val, u32 shift, bool s);
    inline u32 aluBarrelShifter(State* cpu, bool i, bool r, u32 op2, int r15Off);

    // mul functions
    inline void mul32SetNZCFlags(State* cpu, u32 res);
    inline void mul64SetNZCFlags(State* cpu, u64 res);
    inline u32 mulGetICycles(u32 op, bool a);

    // arm instructions
    typedef void (*ArmInstruction)(struct State*, u32);
    namespace Arm {

        void bl(State* cpu, u32 instruction);
        template <bool thumb> void bx(State* cpu, u32 instruction);
        template <bool thumb> void blx_reg(State* cpu, u32 instruction);
        
        template <bool thumb> void alu(State* cpu, u32 instruction);
        void mul(State* cpu, u32 instruction);
        void mull(State* cpu, u32 instruction);
        void smul(State* cpu, u32 instruction);
        void clz(State* cpu, u32 instruction);
        void qadd(State* cpu, u32 instruction);
        void mrs(State* cpu, u32 instruction);
        template <bool thumb> void msr(State* cpu, u32 instruction);
        void ldr_str(State* cpu, u32 instruction);
        void ldrh_strh(State* cpu, u32 instruction);
        template <bool thumb> void ldm_stm(State* cpu, u32 instruction);
        void swp(State* cpu, u32 instruction);
        void swi(State* cpu, u32 instruction);
        void bkpt(State* cpu, u32 instruction);
        void mcr_mrc(State* cpu, u32 instruction);
        void ldc_stc(State* cpu, u32 instruction);
        void mcrr_mrrc(State* cpu, u32 instruction);
        void und(State* cpu, u32 instruction);
        void DEBUG_noop(State* cpu, u32 instruction);

        ArmInstruction decode(State* cpu, u32 instruction);

    }

    // thumb instructions
    typedef void (*ThumbInstruction)(struct State*, u16);
    namespace Thumb {

        void moveShiftedRegister(State* cpu, u16 instruction);
        void addSubtract(State* cpu, u16 instruction);
        void moveCompareAddSubtractImmediate(State* cpu, u16 instruction);
        void hiRegisterOperations(State* cpu, u16 instruction);

        void loadPCRelative(State* cpu, u16 instruction);
        void loadStoreWithRegisterOffset(State* cpu, u16 instruction);
        void loadStoreSignExtendedByteHalfword(State* cpu, u16 instruction);
        void loadStoreWithImmediateOffset(State* cpu, u16 instruction);
        void loadStoreHalfword(State* cpu, u16 instruction);
        void loadStoreSPRelative(State* cpu, u16 instruction);

        ThumbInstruction decode(State* cpu, u16 instruction);
    }
}