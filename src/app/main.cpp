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
#include "../core/arm/arm7.h"

int main(int argc, char* argv[]) {
	Arm7::State arm7;
	arm7.writeReg(420, 0);
	print(arm7.readReg(0));

	std::cout << "Hello FUCK YOU!" << std::endl;
}