#include "../../../math.h"
#include "../../../helpers.h"
#include "interpreter.h"

namespace Interpreter {
    using namespace Arm;

    inline void mul32SetNZCFlags(State* cpu, u32 res) {
        cpu->cpsr.n = (res >> 31) == 1;
        cpu->cpsr.z = res == 0;
        if (cpu->type == Type::Arm7)
            cpu->cpsr.c = false; // Destroyed by the calculation on ARMv4 only
    }
    inline void mul64SetNZCFlags(State* cpu, u64 res) {
        cpu->cpsr.n = (res >> 63) == 1;
        cpu->cpsr.z = res == 0;
        if (cpu->type == Type::Arm7)
            cpu->cpsr.c = false;
    }

    // calculate i cycles
    // TODO: probably incorrect
    inline u32 mulGetICycles(u32 op, bool a) {
        int m;
        if (op & 0x8000'0000) {
            if (op == 0xffff'ffff)
                m = 1;
            else
                m = (31 - __clz(~op ^ 0x8000'0000) + 7) / 8;
        }
        else {
            if (op == 0)
                m = 1;
            else
                m = (31 - __clz(op) + 7) / 8;
        }
        assert(m != 0);
        m += a;

        return m;
    }
}