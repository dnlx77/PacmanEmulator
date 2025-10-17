#pragma once
#include <cstdint>
#include "Memory/MemoryBus.h"

union RegisterPair {
	uint16_t pair;
	struct {
		uint8_t low;
		uint8_t high;
	};
};

static const uint8_t FLAG_C = 0x01;  // Carry
static const uint8_t FLAG_N = 0x02;  // Add/Subtract
static const uint8_t FLAG_PV = 0x04;  // Parity/Overflow
static const uint8_t FLAG_H = 0x10;  // Half Carry
static const uint8_t FLAG_Z = 0x40;  // Zero
static const uint8_t FLAG_S = 0x80;  // Sign

class Z80 {
private:
	// Memory bus
	MemoryBus *m_memory;

	// Registri principali
	RegisterPair BC, DE, HL;
	uint8_t A, F;

	// Shadow regisdters
	RegisterPair BC_alt, DE_alt, HL_alt;
	uint8_t A_alt, F_alt;

	// Registri speciali
	uint16_t PC, SP, IX, IY;
	uint8_t	I, R;

	// Cycles tracking
	uint64_t m_totalCycles;
	int m_cyclesLastInstruction;

	// Opcode table
	using OpcodeFunction = void (Z80::*)();
	OpcodeFunction m_opcodeTable[256];

	void InitOpcodeTable();

	// Opcodes
	void OP_NotImplemented();
	void OP_NOP();
	void OP_INC_A();
	void OP_LD_A_n();
	void OP_LD_B_C();

public:
	Z80(MemoryBus *memory);
	
	void Reset();
	void SetFlag(uint8_t flag, bool value);
	bool GetFlag(uint8_t flag) const;
	void SetPC(uint16_t value);

	void ExchangeAF();  // Swap A,F con A',F'
	void ExchangeAll(); // Swap BC,DE,HL con BC',DE',HL' (istruzione EXX)

	int Step();

	uint64_t GetTotalCycles() const { return m_totalCycles; }
	void ResetCycles() { m_totalCycles = 0; }

	// Getter per debugging (opzionali, aggiungili se/quando servono)
	uint8_t GetA() const { return A; }
	uint8_t GetF() const { return F; }
	uint16_t GetBC() const { return BC.pair; }
	uint16_t GetPC() const { return PC; }
	uint16_t GetSP() const { return SP; }
	// ... altri getter se servono
};