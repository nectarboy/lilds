#include "../../../helpers.h"
#include "../arm.h"
#include "interpreter.h"

namespace Interpreter {
    using namespace Arm;

    bool evalConditionCode(State* cpu, CC cc) {
        switch (cc) {
            case CC::Z_SET: return cpu->cpsr.z;
            case CC::Z_CLR: return !cpu->cpsr.z;
            case CC::C_SET: return cpu->cpsr.c;
            case CC::C_CLR: return !cpu->cpsr.c;
            case CC::N_SET: return cpu->cpsr.n;
            case CC::N_CLR: return !cpu->cpsr.n;
            case CC::V_SET: return cpu->cpsr.v;
            case CC::V_CLR: return !cpu->cpsr.v;
            case CC::C_SET_AND_Z_CLR: return cpu->cpsr.c && !cpu->cpsr.z;
            case CC::C_CLR_OR_Z_SET: return !cpu->cpsr.c || cpu->cpsr.z;
            case CC::N_EQ_V: return cpu->cpsr.n == cpu->cpsr.v;
            case CC::N_NEQ_V: return cpu->cpsr.n != cpu->cpsr.v;
            case CC::Z_CLR_AND_N_EQ_V: return !cpu->cpsr.z && (cpu->cpsr.n == cpu->cpsr.v);
            case CC::Z_SET_OR_N_NEQ_V: return cpu->cpsr.z || (cpu->cpsr.n != cpu->cpsr.v);
            case CC::AL: return true;
            case CC::UND: return false;
            default: std::cout << "[!] UNDEFINED CONDITION CODE: " << (int)cc << "\n"; assert(0); return false;
        }
    }

}