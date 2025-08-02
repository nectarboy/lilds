//#include "src/helpers.h"
//#include "src/math.h"

#include "arm.h"
#include "memory.inl"
#include "interpreter/interpreter.h"
#include "../bus/bus.h"

namespace Arm {

// register methods
u32 State::readReg(int r) {
    return reg[r];
}
void State::writeReg(int r, u32 val) {
    reg[r] = val;
    if (r == 15) {
        issuePipelineFlush();
    }
}

u32 State::readCPSR() {
    u32 val = (int)(cpsr.mode) | (cpsr.t << 5) | (cpsr.f << 6) | (cpsr.i << 7) | (cpsr.v << 28) | (cpsr.c << 29) | (cpsr.z << 30) | (cpsr.n << 31);
    if (type == Type::Arm9) {
        val |= (cpsr.q << 27);
    }
    return val;
}
void State::writeCPSR(u32 val) {
    setMode(Mode(val & 0b11111));
    setThumb((val >> 5) & 1);
    cpsr.f = (val >> 6) & 1;
    cpsr.i = (val >> 7) & 1;
    cpsr.q = (val >> 27) & 1;
    cpsr.v = (val >> 28) & 1;
    cpsr.c = (val >> 29) & 1;
    cpsr.z = (val >> 30) & 1;
    cpsr.n = (val >> 31) & 1;
}

u32 State::readSPSR(int bank) {
    if (bank == 0) {
        return readCPSR();
    }
    else {
        return spsr[bank];
    }
}
void State::writeSPSR(u32 val, int bank) {
    if (bank != 0) {
        if (type == Type::Arm7)
            val &= 0xf000'00ff;
        else
            val &= 0xf800'00ff;
        spsr[bank] = val;
    }
}

void State::copyCPSRToSPSR() {
    writeSPSR(readCPSR(), getModeBank(cpsr.mode));
}
void State::copySPSRToCPSR() {
    int bank = getModeBank(cpsr.mode);
    if (bank != 0) {
        writeCPSR(spsr[bank]);
    }
}

// mode methods
void State::setMode(Mode mode) {
    int bank = getModeBank(mode);
    if (bank == -1) {
        printf("invalid mode (%x) \n", mode);
        lilds__crash();
    }

    Mode oldmode = cpsr.mode;
    int oldbank = getModeBank(oldmode);

    cpsr.mode = mode;

    // FIQ mode reg bank switch
    if (bank == 1 || oldbank == 1) {
        for (int i = 0; i < 5; i++) {
            bankedreg[oldbank][i] = reg[8 + i];
            reg[8 + i] = bankedreg[bank][i];
        }
    }

    // Regular reg bank switch
    bankedreg[oldbank][6] = reg[13];
    bankedreg[oldbank][7] = reg[14];
    reg[13] = bankedreg[bank][6];
    reg[14] = bankedreg[bank][7];

}
void State::setThumb(bool t) {
    if (cpsr.t != t) {
        cpsr.t = t;
        issuePipelineFlush();
    }
}

// execution methods
void State::execute() {
    evenClock = !evenClock;

    // TODO: (Arm9) implement seperate data fetch and code fetch waitstates.
    // Specifically, code fetch waitstates stall the pipeline, while data fetch waitstates don't, due to seperate code and data paths.
    if (waitstates) {
        waitstates--;
        cycles++;
    }
    else {
        // Pipeline refill (can we do the two prefetches immediately without observable effect?)
        if (pipelineStage != 2) {
            if (pipelineStage == 0) {
                if (!cpsr.t) 
                    reg[15] &= 0xffff'fffc;
                else
                    reg[15] &= 0xffff'fffe;
                pipelineFetch<0, Access::N>(cpsr.t);
                pipelineStage++;
            }
            else {
                pipelineFetch<1, Access::S>(cpsr.t);
                nextInstructionAccessType = Access::S;
                pipelineStage++;
            }
            cycles++;
        }
        // Execution starts
        else if (exeStage == -1) {
            if (exceptionPending) {
                handleException();
                exceptionPending = false;
            }
            else {
                currentInstruction = pipeline[0];
                pipeline[0] = pipeline[1];

                exeStage = 0;
                if (!cpsr.t) {
                    pipeline[1] = readCode32(reg[15], nextInstructionAccessType);
                    nextInstructionAccessType = Access::S;

                    Interpreter::ArmInstruction instr = Interpreter::Arm::decode(this, currentInstruction);
                    currentInstructionFun = (void*)instr;
                    instr(this, currentInstruction);

                    if (pipelineStage != 0)
                        reg[15] += 4;
                }
                else {
                    pipeline[1] = readCode16(reg[15], nextInstructionAccessType);
                    nextInstructionAccessType = Access::S;

                    Interpreter::ThumbInstruction instr = Interpreter::Thumb::decode(this, currentInstruction);
                    currentInstructionFun = (void*)instr;
                    instr(this, currentInstruction);

                    if (pipelineStage != 0)
                        reg[15] += 2;
                }
            }
        }
        // Multicycle execution
        else {
            exeStage++;
            if (!cpsr.t) {
                Interpreter::ArmInstruction instr = static_cast<Interpreter::ArmInstruction>(currentInstructionFun);
                instr(this, currentInstruction);
            }
            else {
                Interpreter::ThumbInstruction instr = static_cast<Interpreter::ThumbInstruction>(currentInstructionFun);
                instr(this, currentInstruction);
            }
        }
    }

    // TODO: theres a second execution stage where exceptions are handled
}
void State::handleException() {

}
void State::init() {
    issuePipelineFlush();
}
void State::finishInstruction() {
    exeStage = -1;
}

// pipeline methods
void State::issuePipelineFlush() {
    // if (pipelineStage != 2) {
    //     printf("pipeline flush issued while the pipeline is refilling. how'd'that happen?\n");
    //     lilds__crash();
    // }

    pipelineStage = 0;
    // if (!cpsr.t) {
    //  reg[15] &= 0xffff'fffc;
    //  pipeline[0] = read32(reg[15], Access::N);
    //  pipeline[1] = read32(reg[15 + 4], Access::S);
    //  reg[15] += 8;
    //  cycles += 2;
    // }
    // else {
    //  reg[15] &= 0xffff'fffe;
    //  pipeline[0] = read16(reg[15], Access::N);
    //  pipeline[1] = read16(reg[15 + 2], Access::S);
    //  reg[15] += 4;
    //  cycles += 2;
    // }
}

template <int stage, Access access>
inline void State::pipelineFetch(bool thumb) {
    if (!thumb) {
        pipeline[stage] = readCode32(reg[15], access);
        reg[15] += 4;
    }
    else {
        pipeline[stage] = readCode16(reg[15], access);
        reg[15] += 2;
    }
}

// initialization methods
void State::initialize() {
    cpsr.t = false;
    cpsr.mode = Mode::User;
    setMode(Mode::User);
}
void State::sideLoadAt(u32 addr) {
    initialize();
    writeReg(15, addr);
}

// debug methods
std::string State::getTypeString() {
    if (this->type == Type::Arm7)
        return "Arm7"; // "ARM7TDMI"
    else
        return "Arm9"; // "ARM946E-S"
}
bool State::canPrint() {
    return type == Type::Arm7;
}

}