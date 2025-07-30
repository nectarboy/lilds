#include "../../../helpers.h"
#include "../arm.h"
#include "interpreter.h"

namespace Interpreter {
    using namespace Arm;

    namespace Arm {
        ArmInstruction decode(u32 opcode, Type type) {
            return nullptr;
        }
    }

    namespace Thumb {
        ThumbInstruction decode(u16 opcode, Type type) {
            return nullptr;
        }
    }
}