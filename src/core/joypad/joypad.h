#pragma once
#include "../../helpers.h"

namespace DS {
    struct State;
}

namespace Joypad {

    enum class Key {
        A = 0,
        B = 1,
        Select = 2,
        Start = 3,
        Right = 4,
        Left = 5,
        Up = 6,
        Down = 7,
        ShoulderR = 8,
        ShoulderL = 9,
        X = 10,
        Y = 11
    };

    struct State {
        State() {}

        // references
        DS::State* ds = nullptr;

        // initialization methods
        void initialize();

        // registers
        u16 keysBitmap;
        inline void setKey(Key key, bool pressed) {
            if (pressed)
                keysBitmap &= ~(1 << (int)key);
            else
                keysBitmap |= (1 << (int)key);
        }

        inline u16 getIOPart_KeyInput() {
            return keysBitmap & 0x1ff;
        }
        inline u16 getIOPart_ExtKeyIn() {
            return (keysBitmap >> 9) & 0b11;
        }

        // input methods
        void setRegistersWithKeyboard(std::map<int, bool>& keyboard);
    };

}