#include "GUI.h"

GUI::GUI(RISC& risc) : risc(risc) {
	initscr();
	cbreak();
	noecho();
	nodelay(stdscr, true);
	curs_set(0);

	start_color();
	use_default_colors();
	init_pair(IN_PAIR, COLOR_GREEN, -1);
	init_pair(OUT_PAIR, COLOR_RED, -1);
	init_pair(SPECIAL_PAIR, COLOR_BLUE, -1);

	quit = false;

	Module* instrCtrMod = new Module;
	instrCtrMod->dataWidth = Module::WIDTH_8;
	instrCtrMod->data8 = &risc.instrCtr;
	instrCtrMod->label = "INSTR CTR";
	modules.push_back(instrCtrMod);

	Module* prgrmCtrMod = new Module;
	prgrmCtrMod->dataWidth = Module::WIDTH_16;
	prgrmCtrMod->data16 = &risc.prgrmCtr;
	prgrmCtrMod->signalIn = CS_CI;
	prgrmCtrMod->signalOut = CS_CO;
	prgrmCtrMod->signalSpecial = CS_CE;
	prgrmCtrMod->label = "PRGRM CTR";
	modules.push_back(prgrmCtrMod);

	Module* instrRegMod = new Module;
	instrRegMod->dataWidth = Module::WIDTH_16;
	instrRegMod->data16 = &risc.instrReg;
	instrRegMod->signalIn = CS_II;
	instrRegMod->signalOut = CS_IO;
	instrRegMod->label = "INSTR REG";
	modules.push_back(instrRegMod);

	Module* memAddrRegMod = new Module;
	memAddrRegMod->dataWidth = Module::WIDTH_16;
	memAddrRegMod->data16 = &risc.memAddrReg;
	memAddrRegMod->signalIn = CS_MI;
	memAddrRegMod->signalOut = CS_END;
	memAddrRegMod->label = "MEM ADDR REG";
	modules.push_back(memAddrRegMod);

	Module* memLowerBufMod = new Module;
	memLowerBufMod->dataWidth = Module::WIDTH_8;
	memLowerBufMod->data8 = &risc.memLowerBuf;
	memLowerBufMod->signalIn = CS_ROL | CS_RI;
	memLowerBufMod->signalOut = CS_RO | CS_RIL;
	memLowerBufMod->label = "MEM LOWER BUF";
	modules.push_back(memLowerBufMod);

	Module* ramMod = new Module;
	ramMod->label = "RAM";
	ramMod->signalIn = CS_RI | CS_RIL;
	ramMod->signalOut = CS_ROL | CS_RO;
	ramMod->drawSpecial = true;
	ramMod->drawFunc = [](Module* module, RISC& risc, int x, int y) -> int {
		bool in = risc.controlSignals & module->signalIn;
		bool out = risc.controlSignals & module->signalOut;
		
		if (in) 	attron(COLOR_PAIR(IN_PAIR));
		if (out) 	attron(COLOR_PAIR(OUT_PAIR));
		mvwaddstr(stdscr, y, x, module->label);
		if (in) 	attroff(COLOR_PAIR(IN_PAIR));
		if (out) 	attroff(COLOR_PAIR(OUT_PAIR));

		mvwaddstr(stdscr, y + 1, x, "...");
		
		for (int j = 0; j < 4; ++j) {
			if ((j == 0 && (risc.controlSignals & (CS_RIL | CS_ROL))) || 
				(j == 1 && (risc.controlSignals & (CS_RI  | CS_RO )))) {
				if (in) 	attron(COLOR_PAIR(IN_PAIR));
				if (out) 	attron(COLOR_PAIR(OUT_PAIR));
			}

			mvprintw(y + 2 + j, x, "0x%02x: ", (risc.memAddrReg << 1) + j);
			for (int i = 7; i >= 0; --i) {
				addch((risc.RAM[(risc.memAddrReg << 1) + j] & (1UL << i)) ? '1' : '0');
			}

			if (in) 	attroff(COLOR_PAIR(IN_PAIR));
			if (out) 	attroff(COLOR_PAIR(OUT_PAIR));
		}

		attroff(COLOR_PAIR(IN_PAIR));
		attroff(COLOR_PAIR(OUT_PAIR));

		return 7;
	};
	modules.push_back(ramMod);

	Module* regsMod = new Module;
	regsMod->label = "REGISTERS";
	regsMod->signalIn = CS_XI | CS_YI;
	regsMod->signalOut = CS_XO | CS_YO;
	regsMod->drawSpecial = true;
	regsMod->drawFunc = [](Module* module, RISC& risc, int x, int y) -> int {
		bool in = risc.controlSignals & module->signalIn;
		bool out = risc.controlSignals & module->signalOut;

		if (in) 	attron(COLOR_PAIR(IN_PAIR));
		if (out) 	attron(COLOR_PAIR(OUT_PAIR));
		mvwaddstr(stdscr, y, x, module->label);
		if (in) 	attroff(COLOR_PAIR(IN_PAIR));
		if (out) 	attroff(COLOR_PAIR(OUT_PAIR));

		for (int j = 0; j < 8; ++j) {
			if ((risc.controlSignals & CS_XI && risc.rr.rgx == j) ||
				(risc.controlSignals & CS_YI && risc.rr.rgy == j))
				attron(COLOR_PAIR(IN_PAIR));

			if ((risc.controlSignals & CS_XO && risc.rr.rgx == j) ||
				(risc.controlSignals & CS_YO && risc.rr.rgy == j))
				attron(COLOR_PAIR(OUT_PAIR));

			if (j == 0) {
				mvprintw(y + 1 + j, x, "RG0: ");
			} else {
				mvprintw(y + 1 + j, x, "RG%c: ", 'A' + j - 1);
			}
			
			for (int i = 15; i >= 0; --i) {
				addch((risc.rgs[j] & (1UL << i)) ? '1' : '0');
			}

			if (in) 	attroff(COLOR_PAIR(IN_PAIR));
			if (out) 	attroff(COLOR_PAIR(OUT_PAIR));
		}

		return 10;
	};
	modules.push_back(regsMod);

	Module* rgxMod = new Module;
	rgxMod->dataWidth = Module::WIDTH_16;
	rgxMod->data16 = &risc.rgx;
	rgxMod->signalIn = CS_AX;
	rgxMod->signalSpecial = CS_AD;
	rgxMod->label = "RGX";
	modules.push_back(rgxMod);

	Module* rgyMod = new Module;
	rgyMod->dataWidth = Module::WIDTH_16;
	rgyMod->data16 = &risc.rgy;
	rgyMod->signalIn = CS_AY;
	rgyMod->signalSpecial = CS_AD;
	rgyMod->label = "RGY";
	modules.push_back(rgyMod);

	Module* aluMod = new Module;
	aluMod->dataWidth = Module::WIDTH_16;
	aluMod->data16 = &risc.alu;
	aluMod->signalOut = CS_AD;
	aluMod->signalSpecial = CS_AY | CS_AX;
	aluMod->label = "ALU";
	modules.push_back(aluMod);

	risc.clock();
}

GUI::~GUI() {
	endwin();
};

void GUI::update() {
	w = getmaxx(stdscr);
	h = getmaxy(stdscr);
	ch = getch();

	werase(stdscr);

	if (ch == 'q' | ch == 'Q') {
		quit = true;
		return;
	} else if (ch == 's' || ch == ' ') {
		risc.clock();
	}

	// DRAW CONTROL SIGNALS
	for (int i = 0; i < 32; ++i) {
		mvwaddstr(stdscr, i + 1, 0, getCtrlSignalName(static_cast<CtrlSig>(1UL << i)));
		waddch(stdscr, ' ');
		waddch(stdscr, (risc.controlSignals & (1UL << i)) ? '1' : '0');
	}

	// DRAW DATA BUS
	wmove(stdscr, h/2, w/2 - 8);
	for (int i = 15; i >= 0; --i) {
		addch((risc.dataBus & (1UL << i)) ? '1' : '0');
	}

	int x = 8;
	int y = 1;
	for (auto& module : modules) {
		if (module->drawSpecial) {
			y += module->drawFunc(module, risc, x, y);
			continue;
		}

		wmove(stdscr, y++, x);
		if (risc.controlSignals & module->signalIn) attron(COLOR_PAIR(IN_PAIR));
		if (risc.controlSignals & module->signalOut) attron(COLOR_PAIR(OUT_PAIR));
		if (risc.controlSignals & module->signalSpecial) attron(COLOR_PAIR(SPECIAL_PAIR));
		waddstr(stdscr, module->label);
		wmove(stdscr, y++, x);
		if (module->dataWidth == Module::WIDTH_8) {
			for (int i = 7; i >= 0; --i) {
				addch((*(module->data8) & (1UL << i)) ? '1' : '0');
			}
		} else if (module->dataWidth == Module::WIDTH_16) {
			for (int i = 15; i >= 0; --i) {
				addch((*(module->data16) & (1UL << i)) ? '1' : '0');
			}
		} else if (module->dataWidth == Module::WIDTH_32) {
			for (int i = 31; i >= 0; --i) {
				addch((*(module->data32) & (1UL << i)) ? '1' : '0');
			}
		}
		attroff(COLOR_PAIR(IN_PAIR));
		attroff(COLOR_PAIR(OUT_PAIR));
		attroff(COLOR_PAIR(SPECIAL_PAIR));
		++y;
	}
	
}

const char* GUI::getCtrlSignalName(CtrlSig sig) {
	switch (sig) {
		case CS_CO: 	return "CO ";
		case CS_CI: 	return "CI ";
		case CS_CE: 	return "CE ";
		case CS_MI: 	return "MI ";
		case CS_ROL: 	return "ROL";
		case CS_RO: 	return "RO ";
		case CS_RIL: 	return "RIL";
		case CS_RI: 	return "RI ";
		case CS_II:		return "II ";
		case CS_IO: 	return "IO ";
		case CS_XI:		return "XI ";
		case CS_XO:		return "XO ";
		case CS_YI:		return "YI ";
		case CS_YO:		return "YO ";
		case CS_HLT:	return "HLT";
		default: 		return "---";
	}
}