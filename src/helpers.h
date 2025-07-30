#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <iostream>
#include <fstream>
#include <map>
#include <array>
#include <vector>
#include <bitset>
#include <algorithm>
#include <cassert>

typedef unsigned int uint;
typedef std::uint8_t u8;
typedef std::uint16_t u16;
typedef std::uint32_t u32;
typedef std::uint64_t u64;
typedef std::int8_t s8;
typedef std::int16_t s16;
typedef std::int32_t s32;
typedef std::int64_t s64;

#define __clz(x) __lzcnt(x)
#define __unreachable() assume(0);

#define lenOfArray(arr) (sizeof(arr) / sizeof(arr[0]))

#define print(x) std::cout << (x) << "\n"
#define printBits(x, n) std::cout << std::bitset<n>(x) << "\n"
#define printAndCrash(x) print(x); assert(0);
#define printfAndCrash(x) printf(x); assert(0);

std::vector<char> getFileBinaryVector(std::string fileName);