#pragma once
#include "../../helpers.h"
#include "../ds.h"

// ARM7 is ARM7TDMI, implements ARMv4
// ARM9 is ARM946E-S, implements ARMv5TE

// this namespace contains structs of the arm7 internal state
namespace Arm {

    enum class Type {
        Arm7,
        Arm9
    };

    enum class Mode {
        User = 0b10000,
        System = 0b11111,
        FIQ = 0b10001,
        IRQ = 0b10010,
        Supervisor = 0b10011,
        Abort = 0b10111,
        Undefined = 0b11011
    };

    enum class Access {
        S,
        N
    };

    enum class AccessType {
        Code,
        Data,
    };

    enum class AccessWidth {
        Bus16,
        Bus32
    };

    enum class Exception {
        Reset,
        UndefinedInstruction,
        SWI,
        PrefetchAbort,
        DataAbort,
        AddressExceeds26Bits,
        NormalInterrupt,
        FastInterrupt
    };

    namespace {
        struct CPSR {
            Mode mode = (Mode)0;
            bool t = false;
            bool f = false;
            bool i = false;
            bool q = false;
            bool v = false;
            bool c = false;
            bool z = false;
            bool n = false;
        };
    }

    struct State {
    public:
        State(Type type) {
            this->type = type;
        }

        // register methods
        u32 readReg(int r);
        void writeReg(int r, u32 val);

        u32 readCPSR();
        void writeCPSR(u32 val);

        u32 readSPSR(int bank);
        void writeSPSR(u32 val, int bank);

        void copyCPSRToSPSR();
        void copySPSRToCPSR();

        // memory methods
        inline u8 read8(u32 addr, Access access);
        inline u16 read16(u32 addr, Access access);
        inline u32 read32(u32 addr, Access access);
        inline void write8(u32 addr, u8 val, Access access);
        inline void write16(u32 addr, u16 val, Access access);
        inline void write32(u32 addr, u32 val, Access access);
        inline u16 readCode16(u32 addr, Access access);
        inline u32 readCode32(u32 addr, Access access);

        // waitstate methods
        template <AccessType type, AccessWidth width> inline void addMainMemoryWaitstates9(Access access);
        template <AccessType type, AccessWidth width> inline void addSharedMemoryWaitstates9(Access access);
        template <AccessType type, AccessWidth width> inline void addVRAMWaitstates9(Access access);
        template <AccessType type, AccessWidth width> inline void addTCMCacheWaitstates9(Access access);

        // mode methods
        void setMode(Mode val);
        void setThumb(bool val);

        // execution methods
        void execute();
        void handleException();
        void init();
        void finishInstruction();

        // pipeline methods
        void issuePipelineFlush();
        template <int stage, Access access>
        inline void pipelineFetch(bool thumb);

        // debug methods
        std::string getTypeString();

        // static stuff
        static inline int getModeBank(Mode mode) {
            switch (mode) {
                case Mode::User:
                case Mode::System: return 0;
                case Mode::FIQ: return 1;
                case Mode::IRQ: return 2;
                case Mode::Supervisor: return 3;
                case Mode::Abort: return 4;
                case Mode::Undefined: return 5;
                default: return -1;
            }
        }

        Type type;

        // references
        DS::State* ds = nullptr;

        // registers
        CPSR cpsr;
        u32 reg[16];
        //u32 oldr15 = 0;
        u32 bankedreg[6][7];
        u32 spsr[6];

        // public state for now
        int exeStage = -1;
        u32 tmp[16]; // used in instruction functions
        u32 cycles = 0;
        bool evenClock = false;
        u32 waitstates = 0;
        Access nextInstructionAccessType = Access::S;

    private:
        // private state
        u32 pipeline[2];
        int pipelineStage = 0;
        u32 currentInstruction = 0;
        void* currentInstructionFun = nullptr;
        bool exceptionPending = false;
        Exception exceptionType;
    };

}