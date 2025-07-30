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

//#include "../helpers.h"
#include "../core/ds.h"
#include "../core/arm/arm.h"
#include "../core/bus/bus.h"

int main(int argc, char* argv[]) {
    std::cout << __DATE__ << "\t" << __TIME__ << std::endl;

    DS::State lilds;
    static_cast<Arm::State*>(lilds.arm7)->writeReg(0, 420);
    print(static_cast<Arm::State*>(lilds.arm7)->getTypeString());
    print(static_cast<Arm::State*>(lilds.arm9)->getTypeString());

    std::cout << "Hello!" << std::endl;

    return 0;
}