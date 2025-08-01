#include "helpers.h"

std::vector<char> getFileBinaryVector(std::string filePath) {
	std::ifstream file(filePath, std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		printAndCrash("File not found... Exiting.");
	}
	u64 size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<char> bin(size);
	file.read(bin.data(), size);

	return bin;
}