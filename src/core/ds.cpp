#include "ds.h"
#include "arm/arm.h"
#include "bus/bus.h"

namespace DS {

    // initialization methods
    void State::createComponents() {
        Arm::State* _arm7 = static_cast<Arm::State*>(arm7);
        Arm::State* _arm9 = static_cast<Arm::State*>(arm9);
        Bus::State* _bus = static_cast<Bus::State*>(bus);

        if (_arm7) delete _arm7;
        _arm7 = new Arm::State(Arm::Type::Arm7);    
        _arm7->ds = this;

        if (_arm9) delete _arm9;
        _arm9 = new Arm::State(Arm::Type::Arm9);    
        _arm9->ds = this;

        if (_bus) delete _bus;
        _bus = new Bus::State();    
        _bus->ds = this;

        arm7 = _arm7;
        arm9 = _arm9;
        bus = _bus;

        printf("Created DS Components\n");
    }

    void State::initialize() {
        Arm::State* _arm7 = static_cast<Arm::State*>(arm7);
        Arm::State* _arm9 = static_cast<Arm::State*>(arm9);
        Bus::State* _bus = static_cast<Bus::State*>(bus);

        _arm7->initialize();
        _arm9->initialize();
        _bus->initialize();
    }

    // loading methods
    void State::loadRomFileIntoMainMem(std::vector<char>& romFile) {
        Arm::State* _arm7 = static_cast<Arm::State*>(arm7);
        Arm::State* _arm9 = static_cast<Arm::State*>(arm9);
        Bus::State* _bus = static_cast<Bus::State*>(bus);

        _bus->loadRomFile(romFile);
        _bus->sideLoadRomToMainMem(_arm7, _arm9);
    }

    // execution methods
    void State::execute() {
        Arm::State* _arm7 = static_cast<Arm::State*>(arm7);
        Arm::State* _arm9 = static_cast<Arm::State*>(arm9);

        _arm7->execute();
        _arm9->execute();
        _arm9->execute();
    }

}