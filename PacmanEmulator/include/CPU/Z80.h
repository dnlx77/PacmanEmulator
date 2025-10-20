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

	// Register mapping array
	uint8_t *m_registerMap[8];

	// Opcode table
	using OpcodeFunction = void (Z80::*)();
	OpcodeFunction m_opcodeTable[256];

	void InitOpcodeTable();

	// Helper per calcolo parità
	bool CalculateParity(uint8_t value);

	// Opcodes
	void OP_NotImplemented();
	void OP_NOP();

	// Opcodes handlers - INC
	void OP_INC_A();
	void OP_INC_B();
	void OP_INC_C();
	void OP_INC_D();
	void OP_INC_E();
	void OP_INC_H();
	void OP_INC_L();

	// Opcodes handlers - DEC
	void OP_DEC_A();
	void OP_DEC_B();
	void OP_DEC_C();
	void OP_DEC_D();
	void OP_DEC_E();
	void OP_DEC_H();
	void OP_DEC_L();

	// Opcodes handlers - LD r, n
	void OP_LD_A_n();
	void OP_LD_B_n();
	void OP_LD_C_n();
	void OP_LD_D_n();
	void OP_LD_E_n();
	void OP_LD_H_n();
	void OP_LD_L_n();

	// Opcodes handelers - ADD A, r
	void OP_ADD_A_A();
	void OP_ADD_A_B();
	void OP_ADD_A_C();
	void OP_ADD_A_D();
	void OP_ADD_A_E();
	void OP_ADD_A_H();
	void OP_ADD_A_L();

	// Opcodes handelers - SUB A, r
	void OP_SUB_A_A();
	void OP_SUB_A_B();
	void OP_SUB_A_C();
	void OP_SUB_A_D();
	void OP_SUB_A_E();
	void OP_SUB_A_H();
	void OP_SUB_A_L();

	//Opcode handler unico per LD r, r'
	void OP_LD_r_r();

	// Opcode handlers - AND A,r
	void OP_AND_A_A(); 
	void OP_AND_A_B(); 
	void OP_AND_A_C(); 
	void OP_AND_A_D(); 
	void OP_AND_A_E(); 
	void OP_AND_A_H(); 
	void OP_AND_A_L(); 

	// Opcode handlers - XOR A,r
	void OP_XOR_A_A();
	void OP_XOR_A_B();
	void OP_XOR_A_C();
	void OP_XOR_A_D();
	void OP_XOR_A_E();
	void OP_XOR_A_H();
	void OP_XOR_A_L();

	// Opcode handlers - OR A,r
	void OP_OR_A_A();
	void OP_OR_A_B();
	void OP_OR_A_C();
	void OP_OR_A_D();
	void OP_OR_A_E();
	void OP_OR_A_H();
	void OP_OR_A_L();

	// Opcode handlers - CP A,r
	void OP_CP_A_A();
	void OP_CP_A_B();
	void OP_CP_A_C();
	void OP_CP_A_D();
	void OP_CP_A_E();
	void OP_CP_A_H();
	void OP_CP_A_L();

	//Opcode HALT
	void OP_HALT();

	// Jump assoluti
	void OP_JP_nn();
	void OP_JP_Z_nn();
	void OP_JP_NZ_nn();
	void OP_JP_C_nn();
	void OP_JP_NC_nn();

	// Jump relativi
	void OP_JR_e();
	void OP_JR_Z_e();
	void OP_JR_NZ_e();
	void OP_JR_C_e();
	void OP_JR_NC_e();

	// Call/Ret
	void OP_CALL_nn();
	void OP_CALL_Z_nn();
	void OP_CALL_NZ_nn();
	void OP_CALL_C_nn();
	void OP_CALL_NC_nn();
	void OP_RET();
	void OP_RET_Z();
	void OP_RET_NZ();
	void OP_RET_C();
	void OP_RET_NC();

	// Push/Pop
	void OP_PUSH_BC();
	void OP_PUSH_DE();
	void OP_PUSH_HL();
	void OP_PUSH_AF();
	void OP_POP_BC();
	void OP_POP_DE();
	void OP_POP_HL();
	void OP_POP_AF();

	// LD rr,nn
	void OP_LD_BC_nn();
	void OP_LD_DE_nn();
	void OP_LD_HL_nn();
	void OP_LD_SP_nn();

	// Inc rr
	void OP_INC_BC();
	void OP_INC_DE();
	void OP_INC_HL();
	void OP_INC_SP();

	// Dec rr
	void OP_DEC_BC();
	void OP_DEC_DE();
	void OP_DEC_HL();
	void OP_DEC_SP();

	// Helper functions per operazioni comuni
	void INC_r(uint8_t &reg);							  // Incremento 8-bit
	void DEC_r(uint8_t &reg);							  // Decremento 8-bit
	void LD_r_n(uint8_t &reg);							  // Load immediate
	void LD_r_r(uint8_t &dest, uint8_t src);			  // Load register to register
	void ADD_A_r(uint8_t value);						  // Addizione
	void SUB_A_r(uint8_t value);						  // Sottrazione
	void AND_A_r(uint8_t value);						  // AND
	void OR_A_r(uint8_t value);							  // OR
	void XOR_A_r(uint8_t value);						  // XOR
	void CP_A_r(uint8_t value);							  // Compare
	void RET_conditional(uint8_t flag, bool condition);   // Ret
	void CALL_conditional(uint8_t flag, bool condition);  // Call
	void JP_nn_conditional(uint8_t flag, bool condition); // Jump nn
	void JR_conditional(uint8_t flag, bool condition);    // Jump e
	void PUSH_rr(RegisterPair const *reg);				  // Push
	void POP_rr(RegisterPair *reg);				          // Pop
	void LD_rr_nn(uint16_t &reg);
	void INC_rr(uint16_t & reg);
	void DEC_rr(uint16_t &reg);

	// Helper functions per stack
	void PUSH_16bit(uint16_t value);
	uint16_t POP_16bit();

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