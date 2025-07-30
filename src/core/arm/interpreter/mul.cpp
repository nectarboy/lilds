#include "../../../math.h"
#include "../../../helpers.h"
#include "interpreter.h"

namespace Interpreter {
    using namespace Arm;

    // calculate i cycles
    // TODO: probably incorrect
    u32 mulGetICycles(u32 op, bool a) {
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
        m += a;

        return m;
    }
}