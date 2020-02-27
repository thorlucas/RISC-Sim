#include <cstdio>
#include <cxxopts.hpp>
#include <string>
#include "RISC.h"
#include "GUI.h"

int main(int argc, char* argv[]) {
	cxxopts::Options options("RISC-Sim", "A simulation of a 16-bit RISC processor");
	options.add_options()
		("r,preload-ram", "Preloads the RAM with the contents of a file.", cxxopts::value<std::string>());

	auto result = options.parse(argc, argv);

	RISC risc;
	GUI gui(risc);

	if (result.count("r") == 1) {
		std::string ramFileName = result["r"].as<std::string>();
		FILE* ramFile = fopen(ramFileName.c_str(), "r");
		uint8_t* ramBuffer = static_cast<uint8_t*>(malloc(RAM_SIZE));

		fread(ramBuffer, sizeof(uint8_t), RAM_SIZE, ramFile);
		risc.copyRAM(ramBuffer);
		
		free(ramBuffer);
		fclose(ramFile);
	}

	while (!gui.quit) {
		gui.update();
	}

	return 0;
}
