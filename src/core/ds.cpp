#include "ds.h"
#include "arm/arm.h"
#include "bus/bus.h"

namespace DS {

    void State::createComponents() {
        if (arm7) delete arm7;
        arm7 = new Arm::State(Arm::Type::Arm7);    
        static_cast<Arm::State*>(arm7)->ds = this;

        if (arm9) delete arm9;
        arm9 = new Arm::State(Arm::Type::Arm9);    
        static_cast<Arm::State*>(arm9)->ds = this;

        if (bus) delete bus;
        bus = new Bus::State();    
        static_cast<Bus::State*>(bus)->ds = this;

        printf("Created DS Components\n");
    }

}