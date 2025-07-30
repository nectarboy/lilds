#pragma once
#include "helpers.h"

inline u64 bitRotateLeft(u64 val, uint n, uint shift);
inline u64 bitRotateRight(u64 val, uint n, uint shift);
inline u64 bitShiftLeft(u64 val, uint n, uint shift);
inline u64 bitShiftRight(u64 val, uint n, uint shift);
inline u64 bitSignedShiftRight(u64 val, uint n, uint shift);

inline void TEST_MATH();