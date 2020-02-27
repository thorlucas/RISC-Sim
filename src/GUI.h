#include <curses.h>
#include <vector>
#include "RISC.h"

#define IN_PAIR 1
#define OUT_PAIR 2
#define SPECIAL_PAIR 3

struct Module {
	enum DataWidth {
		WIDTH_8,
		WIDTH_16,
		WIDTH_32,
	} dataWidth;

	union {		
		uint8_t* data8;
		uint16_t* data16;
		uint32_t* data32;
	};

	uint32_t signalOut 		= CS_END;
	uint32_t signalIn  		= CS_END;
	uint32_t signalSpecial  = CS_END;

	const char* label;

	bool drawSpecial = false;
	int (*drawFunc)(Module*, RISC&, int, int);

	int getWidth() const {
		switch (dataWidth) {
			case WIDTH_8: return 8;
			case WIDTH_16: return 16;
			case WIDTH_32: return 32;
		};
	}
};

class GUI {
private:
	int ch;
	int w, h;

	const char* getCtrlSignalName(CtrlSig sig);

public:
	std::vector<Module*> modules;

	RISC& risc;
	bool quit;

	GUI(RISC& risc);
	~GUI();

	void update();
};