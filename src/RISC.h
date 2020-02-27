#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>

// TODO: Change these sizes to actual size
#define RAM_SIZE 1UL << 12
#define FLASH_SIZE 1UL << 14

enum CtrlSig : uint32_t {
	CS_CO 	= 1UL << 0	, /**< Program counter out to bus. */
	CS_CI 	= 1UL << 1	, /**< Program counter read in from bus. */
	CS_CE 	= 1UL << 2	, /**< Increments program counter. */
	CS_MI 	= 1UL << 3	, /**< Stores address on bus to memory address register. */
	CS_ROL 	= 1UL << 4	, /**< Reads the lower byte of RAM onto lower buffer. */
	CS_RO 	= 1UL << 5 	, /**< Outputs the upper byte from RAM and lower byte from buffer. */
	CS_RIL  = 1UL << 6	, /**< Inputs the lower byte to buffer from bus. */
	CS_RI 	= 1UL << 7 	, /**< Inputs the upper byte from bus and lower byte from buffer. */
	CS_II 	= 1UL << 8	, /**< Reads to instruction register from bus. */
	CS_IO 	= 1UL << 9 	, /**< Writes the lower 8 bits of instruction register (imm) to bus. */
	CS_XI 	= 1UL << 10 	, /**< Reads into X register from the bus. */
	CS_XO 	= 1UL << 11 ,
	CS_YI 	= 1UL << 12 ,
	CS_YO	= 1UL << 13	,

	CS_HLT 	= 1UL << 30	, /**< Ends the current instruction cycle early. */
	CS_END	= 1UL << 31	,
};

enum Flag : uint32_t {

};

enum InstrCt : uint8_t {
	IC_0 	= 1UL << 0	,
	IC_1 	= 1UL << 1	,
	IC_2 	= 1UL << 2 	,
	IC_3 	= 1UL << 3	,
	IC_4 	= 1UL << 4	,
	IC_5 	= 1UL << 5	,
	IC_END 	= 1UL << 6	,
};

class RISC {
friend class GUI;
protected:
	uint8_t* RAM;
	uint8_t* FLASH;

	/**
	 * Stores the short word address being accessed.
	 * 
	 * The least significant bit is controlled independently and
	 * represents which byte of the word is currently being accessed.
	 */
	uint16_t memAddrReg;

	/**
	 * Stores the lower 8 bits of a 16 bit word.
	 * 
	 * During a read cycle, this buffer needs to first be loaded with
	 * the lower byte, then in the next tick the upper byte is read
	 * while this byte is output.
	 * During a write cycle, the lower byte is latched to this while
	 * the upper is written to RAM, and in the next tick this is
	 * written to the RAM.
	 */
	uint8_t memLowerBuf;
	uint8_t memInternalBus; /**< The internal bus from RAM to the buffer. */

	union {
		uint16_t instrReg; /**< Stores the current instruction to be executed. */
		struct {
			unsigned 	: 12;
			unsigned i1 : 1	;
			unsigned i2 : 1	;
			unsigned i4 : 1	;
			unsigned i8 : 1	;
		};
		struct { // RI types
			unsigned imm: 8;
			unsigned rgx: 3;
			unsigned 	: 5;
		} ri;
		struct { // RR types
			unsigned rgy: 3;
			unsigned 	: 5;
			unsigned rgx: 3;
			unsigned 	: 5;
		} rr;
		struct { // Imm types
			unsigned imm: 8;
			unsigned 	: 8;
		} imm;
	};

	uint8_t instrCtr; /**< Instruction decade counter. */
	uint16_t prgrmCtr; /**< Program counter. */

	uint16_t rgs[8]; /**< The eight registers. */
	uint16_t regInternalOutBus;
	uint16_t regInternalInBus;

	void clockFallingEdge();
	void clockRisingEdge();

	void setCtrlSig(CtrlSig sig, bool bit);
	void setFlag(Flag flag, bool bit);

public:
	uint16_t dataBus;
	uint32_t controlSignals;
	uint32_t flags;

	void copyRAM(uint8_t* src);

	RISC();
	~RISC();

	void clock();
};