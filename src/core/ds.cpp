#include "ds.h"
#include "arm/arm.h"
#include "bus/bus.h"
#include "joypad/joypad.h"

namespace DS {

    // initialization methods
    void State::createComponents() {
        Arm::State* _arm7 = static_cast<Arm::State*>(arm7);
        Arm::State* _arm9 = static_cast<Arm::State*>(arm9);
        Bus::State* _bus = static_cast<Bus::State*>(bus);
        Joypad::State* _joypad = static_cast<Joypad::State*>(joypad);

        if (_arm7) delete _arm7;
        _arm7 = new Arm::State(Arm::Type::Arm7);    
        _arm7->ds = this;

        if (_arm9) delete _arm9;
        _arm9 = new Arm::State(Arm::Type::Arm9);    
        _arm9->ds = this;

        if (_bus) delete _bus;
        _bus = new Bus::State();    
        _bus->ds = this;

        if (_joypad) delete _joypad;
        _joypad = new Joypad::State();
        _joypad->ds = this;

        arm7 = _arm7;
        arm9 = _arm9;
        bus = _bus;
        joypad = _joypad;

        printf("Created DS Components\n");
    }

    void State::initialize() {
        Arm::State* _arm7 = static_cast<Arm::State*>(arm7);
        Arm::State* _arm9 = static_cast<Arm::State*>(arm9);
        Bus::State* _bus = static_cast<Bus::State*>(bus);
        Joypad::State* _joypad = static_cast<Joypad::State*>(joypad);

        _arm7->initialize();
        _arm9->initialize();
        _bus->initialize();
        _joypad->initialize();
    }

    // frontend methods
    void State::updateJoypadWithKeyboard(std::map<int, bool>& keyboard) {
        Joypad::State* _joypad = static_cast<Joypad::State*>(joypad);
        _joypad->setRegistersWithKeyboard(keyboard);
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