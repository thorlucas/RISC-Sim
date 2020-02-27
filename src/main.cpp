#include <cstdio>
#include "RISC.h"
#include "GUI.h"

int main(int argc, char const* argv[]) {
	RISC risc;
	GUI gui(risc);

	while (!gui.quit) {
		gui.update();
	}

	return 0;
}
