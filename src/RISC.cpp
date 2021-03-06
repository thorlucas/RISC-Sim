#include "RISC.h"

RISC::RISC() {
	RAM = new uint8_t[RAM_SIZE];
	FLASH = new uint8_t[FLASH_SIZE];

	memset(RAM, 0, sizeof(uint8_t) * RAM_SIZE);
	memset(FLASH, 0, sizeof(uint8_t) * FLASH_SIZE);

	dataBus = 0x0;
	controlSignals = 0x0;

	memAddrReg = 0x0;
	memLowerBuf = 0x0;
	memset(rgs, 0, sizeof(uint16_t) * 8);

	prgrmCtr = 0x0;
	instrReg = 0x0;
	instrCtr = IC_5;
}

RISC::~RISC() {
	delete[] RAM;
	delete[] FLASH;
}

void RISC::copyRAM(uint8_t* src) {
	memcpy(RAM, src, sizeof(uint8_t) * RAM_SIZE);
}

void RISC::setCtrlSig(CtrlSig sig, bool bit) {
	if (bit) {
		controlSignals |= sig;
	} else {
		controlSignals &= ~sig;
	}
}

void RISC::setFlag(Flag flag, bool bit) {
	if (bit) {
		flags |= flag;
	} else {
		flags &= ~flag;
	}
}

void RISC::clock() {
	clockFallingEdge();
	clockRisingEdge();
}

void RISC::clockFallingEdge() {
	controlSignals = 0x0;
	dataBus = 0x0;
	memInternalBus = 0x0;

	if (instrCtr == IC_0) {
		setCtrlSig(CS_CO	, true);
		setCtrlSig(CS_MI	, true);
	}

	if (instrCtr == IC_1) {
		setCtrlSig(CS_ROL	, true);
		setCtrlSig(CS_CE 	, true);
	}

	if (instrCtr == IC_2) {
		setCtrlSig(CS_RO	, true);
		setCtrlSig(CS_II 	, true);
	}

	// INSTRUCTIONS IMPLEMENTED:
	// NOP
	// MOV
	// LOD
	// LDI
	// STO
	// ADD
	// JMP

	if (instrCtr == IC_3) {
		setCtrlSig(CS_HLT	, ~i8 & ~i4 & ~i2 & ~i1 	);
		setCtrlSig(CS_YO 	, ~i8 & (
							      ( ~i2 &  i1 )         | 
							      ( ~i4 &  i2 & ~i1 ) )	);
		setCtrlSig(CS_MI	, ~i8 & (
							       (~i4 &  i2 & ~i1) 	|
							       ( i4 & ~i2 &  i1) )	);
		setCtrlSig(CS_IO 	, ~i8 & ~i4 &  i2 &  i1 	);
		setCtrlSig(CS_XI 	, ~i8 & ~i4 &        i1 	);
		setCtrlSig(CS_HLT 	, ~i8 & ~i4 &        i1 	);
		setCtrlSig(CS_XO 	,( i8 &  i4 & ~i2 & ~i1 ) 	|
							 ( i8 & ~i4 & ~i2 & ~i1 )	);
		setCtrlSig(CS_AX 	,( i8 &  i4 & ~i2 & ~i1 )	|
							 ( i8 & ~i4 & ~i2 & ~i1 )	);
	}

	if (instrCtr == IC_4) {
		setCtrlSig(CS_ROL	, ~i8 & ~i4 &  i2 & ~i1 	);
		setCtrlSig(CS_XO 	, ~i8 &  i4 & ~i2 &  i1 	);
		setCtrlSig(CS_RI 	, ~i8 &  i4 & ~i2 &  i1 	);
		setCtrlSig(CS_YO 	,  i8 &  i4 & ~i2 & ~i1 	);
		setCtrlSig(CS_AY 	,( i8 &  i4 & ~i2 & ~i1 )	|
							 ( i8 & ~i4 & ~i2 & ~i1 )	);
		setCtrlSig(CS_IO 	,( i8 & ~i4 & ~i2 & ~i1 )	);
	}

	if (instrCtr == IC_5) {
		setCtrlSig(CS_RO	, ~i8 & ~i4 &  i2 & ~i1 	);
		setCtrlSig(CS_XI	, ~i1 & (
								  ( ~i8 & ~i4 &  i2 ) 	|	
								  (  i8 &  i4 & ~i2 ) ) );
		setCtrlSig(CS_HLT	,(~i8 & ~i4 &  i2 & ~i1)	|
							 (~i8 &  i4 & ~i2 &  i1)	|
							 ( i8 &  i4 & ~i2 & ~i1)	);
		setCtrlSig(CS_RIL 	, ~i8 &  i4 & ~i2 &  i1 	);
		setCtrlSig(CS_AD 	,( i8 &  i4 & ~i2 & ~i1 ) 	|
							 ( i8 & ~i4 & ~i2 & ~i1 )	);
		setCtrlSig(CS_CI 	,( i8 & ~i4 & ~i2 & ~i1 )	);
	}

	if (controlSignals & CS_CO) 	dataBus 		|= prgrmCtr;
	
	if (controlSignals & CS_ROL)	memInternalBus 	|= RAM[memAddrReg << 1];
	if (controlSignals & CS_RO) 	dataBus 		|= (RAM[(memAddrReg << 1) + 1] << 8) + (memLowerBuf << 0);
	if (controlSignals & CS_RIL)	memInternalBus 	|= memLowerBuf;

	if (controlSignals & CS_IO)		dataBus 		|= ri.imm;
	
	if (controlSignals & CS_XO) {
		if (rr.rgx == 0)
			regInternalOutBus 						=  0;
		else
			regInternalOutBus 						=  rgs[rr.rgx];
		dataBus 									|= regInternalOutBus;
	}
	
	if (controlSignals & CS_YO) {
		if (rr.rgy == 0)
			regInternalOutBus 						=  0;
		else
			regInternalOutBus 						=  rgs[rr.rgy];
		dataBus 									|= regInternalOutBus;
	}

	if (controlSignals & CS_AD)		dataBus 		|= rgx + rgy;
}

void RISC::clockRisingEdge() {
	if (controlSignals & CS_CI) 	prgrmCtr 	= dataBus;
	if (controlSignals & CS_ROL)	memLowerBuf	= memInternalBus;
	if (controlSignals & CS_RI) {
		RAM[(memAddrReg << 1) + 1] 				= dataBus >> 8;
		memLowerBuf 							= dataBus & 0x00FF;
	}
	if (controlSignals & CS_RIL) {
		RAM[(memAddrReg << 1) + 0] 				= memLowerBuf;
	}
	if (controlSignals & CS_MI) 	memAddrReg 	= dataBus;
	
	if (controlSignals & CS_II) 	instrReg 	= dataBus;
	if (controlSignals & CS_CE) 	++prgrmCtr;
	
	if (controlSignals & CS_XI)	{
		regInternalInBus 						= dataBus;
		if (rr.rgx != 0)
			rgs[rr.rgx] 						= regInternalInBus;
	}

	if (controlSignals & CS_YI)	{
		regInternalInBus 						= dataBus;
		if (rr.rgy != 0)
			rgs[rr.rgy] 						= regInternalInBus;
	}

	if (controlSignals & CS_AX) 	rgx 		= dataBus;
	if (controlSignals & CS_AY)		rgy			= dataBus;

	if (~(controlSignals & CS_HLT)) instrCtr 	<<= 0x1;
	if (instrCtr == IC_END)			instrCtr 	= IC_0;
	if (controlSignals & CS_HLT)	instrCtr 	= IC_0;

	/** AUTO UPDATE */ 				alu 		= rgx + rgy; 
}