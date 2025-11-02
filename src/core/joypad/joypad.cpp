#include "joypad.h"
#include "SDL.h"

namespace Joypad {

    void State::initialize() {
        keysBitmap = (1 << 12) - 1; // all released
    }

    // TODO: seperate SDL keyboard from ts
    void State::setRegistersWithKeyboard(std::map<int, bool>& keyboard) {
        setKey(Key::A, keyboard[SDLK_x]);
        setKey(Key::B, keyboard[SDLK_z]);
        setKey(Key::Select, keyboard[SDLK_RSHIFT]);
        setKey(Key::Start, keyboard[SDLK_RETURN]);
        setKey(Key::Up, keyboard[SDLK_UP]);
        setKey(Key::Down, keyboard[SDLK_DOWN]);
        setKey(Key::Left, keyboard[SDLK_LEFT]);
        setKey(Key::Right, keyboard[SDLK_RIGHT]);
        setKey(Key::X, keyboard[SDLK_s]);
        setKey(Key::Y, keyboard[SDLK_a]);
    }

}