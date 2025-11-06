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
static const uint8_t FLAG_PV = 0x04; // Parity/Overflow
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
	uint16_t *m_register16Map[4];

	uint8_t m_interruptMode = 1;
	bool m_interruptsEnabled;
	bool pendingInterrupt;
	bool m_halted = false;
	uint8_t m_interruptVector = 0xFF; // Default vector

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
	void OP_ADD_A_HL();

	// Opcodes handelers - SUB A, r
	void OP_SUB_A_A();
	void OP_SUB_A_B();
	void OP_SUB_A_C();
	void OP_SUB_A_D();
	void OP_SUB_A_E();
	void OP_SUB_A_H();
	void OP_SUB_A_L();
	void OP_SUB_A_HL();

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
	void OP_AND_A_HL();

	// Opcode handlers - XOR A,r
	void OP_XOR_A_A();
	void OP_XOR_A_B();
	void OP_XOR_A_C();
	void OP_XOR_A_D();
	void OP_XOR_A_E();
	void OP_XOR_A_H();
	void OP_XOR_A_L();
	void OP_XOR_A_HL();

	// Opcode handlers - OR A,r
	void OP_OR_A_A();
	void OP_OR_A_B();
	void OP_OR_A_C();
	void OP_OR_A_D();
	void OP_OR_A_E();
	void OP_OR_A_H();
	void OP_OR_A_L();
	void OP_OR_A_HL();

	// Opcode handlers - CP A,r
	void OP_CP_A_A();
	void OP_CP_A_B();
	void OP_CP_A_C();
	void OP_CP_A_D();
	void OP_CP_A_E();
	void OP_CP_A_H();
	void OP_CP_A_L();
	void OP_CP_A_HL();

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

	// ADD HL, rr
	void OP_ADD_HL_BC();
	void OP_ADD_HL_DE();
	void OP_ADD_HL_HL();
	void OP_ADD_HL_SP();

	// LD Special
	void OP_LD_A_BC();
	void OP_LD_A_DE();
	void OP_LD_BC_A();
	void OP_LD_DE_A();
	void OP_LD_A_nn();
	void OP_LD_nn_A();

	// Operation Immediate
	void OP_ADD_n();
	void OP_SUB_n();
	void OP_AND_n();
	void OP_XOR_n();
	void OP_OR_n();
	void OP_CP_n();
	void OP_LD_HL_n();

	// ADC A, r
	void OP_ADC_A_B();
	void OP_ADC_A_C();
	void OP_ADC_A_D();
	void OP_ADC_A_E();
	void OP_ADC_A_H();
	void OP_ADC_A_L();
	void OP_ADC_A_HL();
	void OP_ADC_A_A(); 
	void OP_ADC_A_n();

	// SBC A, r
	void OP_SBC_A_B();
	void OP_SBC_A_C();
	void OP_SBC_A_D();
	void OP_SBC_A_E();
	void OP_SBC_A_H();
	void OP_SBC_A_L();
	void OP_SBC_A_HL();
	void OP_SBC_A_A();
	void OP_SBC_A_n();

	// LD 16bit
	void OP_LD_HL_pnn();
	void OP_LD_pnn_HL();
	void OP_LD_SP_HL();

	// CB
	void OP_CB_Prefix();

	// ED
	void OP_ED_Prefix();

	// DD
	void OP_DD_Prefix();

	// FD
	void OP_FD_Prefix();

	// RST
	void OP_RST_00();
	void OP_RST_08();
	void OP_RST_10();
	void OP_RST_18();
	void OP_RST_20();
	void OP_RST_28();
	void OP_RST_30();
	void OP_RST_38();

	void OP_EX_DE_HL();
	void OP_JP_HL();

	void OP_DI();
	void OP_DJNZ();

	void OP_OUT_n_A();
	void OP_EI();
	void OP_JP_M_nn();

	void OP_RETI();

	void OP_RRCA();

	void OP_EXX();

	void OP_CPL();

	void OP_RLCA();

	void OP_INC_pHL();
	void OP_DEC_pHL();
					  
	// Helper functions per operazioni comuni
	void INC_r(uint8_t &reg);								// Incremento 8-bit
	void DEC_r(uint8_t &reg);								// Decremento 8-bit
	void LD_r_n(uint8_t &reg);								// Load immediate
	void LD_r_r(uint8_t &dest, uint8_t src);				// Load register to register
	void ADD_A_r(uint8_t value);							// Addizione
	void SUB_A_r(uint8_t value);							// Sottrazione
	void AND_A_r(uint8_t value);							// AND
	void OR_A_r(uint8_t value);								// OR
	void XOR_A_r(uint8_t value);							// XOR
	void CP_A_r(uint8_t value);								// Compare
	void RET_conditional(uint8_t flag, bool condition);		// Ret
	void CALL_conditional(uint8_t flag, bool condition);	// Call
	void JP_nn_conditional(uint8_t flag, bool condition);	// Jump nn
	void JR_conditional(uint8_t flag, bool condition);		// Jump e
	void PUSH_rr(RegisterPair const *reg);					// Push
	void POP_rr(RegisterPair *reg);							// Pop
	void LD_rr_nn(uint16_t &reg);							// Load 16 bit
	void INC_rr(uint16_t & reg);							// Inc 16 bit
	void DEC_rr(uint16_t &reg);								// Dec 16 bit
	void ADD_HL_rr(uint16_t reg);							// ADD 16 bit
	void ADD_IX_rr(uint16_t reg);							// ADD 16 bit
	void ADD_IY_rr(uint16_t reg);							// ADD 16 bit
	void INC_Memory(uint16_t address);						// Incremente valore in memoria
	void DEC_Memory(uint16_t address);						// Decremente valore in memoria
	void LD_A_indirect(uint16_t address);					// Load da memoria indirizzo registro
	void LD_indirect_A(uint16_t address);					// Store in memoria indirizzo registro
	void LD_A_addr();										// Load da memoria indirizzo istruzione
	void LD_addr_A();										// Store in memoria indirizzo istruzione
	void ADC_A_r(uint8_t value);							// Somma con carry
	void SBC_A_r(uint8_t value);							// Sottrazione con carry
	void HandleRotateShift(uint8_t operation, uint8_t reg); // Rotate shift operation
	void HandleBit(uint8_t bit_number, uint8_t reg);		// Bit operation
	void HandleRes(uint8_t bit_number, uint8_t reg);		// Bit reset
	void HandleSet(uint8_t bit_number, uint8_t reg);		// Bit set
	void HandleRotateShift_pIXOffset(uint8_t operation, int8_t offset);
	void HandleBit_pIXOffset(uint8_t operation, int8_t offset);
	void HandleRes_pIXOffset(uint8_t operation, int8_t offset);
	void HandleSet_pIXOffset(uint8_t operation, int8_t offset);
	void CB_RLC(uint8_t &reg);								// Rotazione a sinistra con carry
	void CB_RRC(uint8_t &reg);								// Rotazione a destra con carry
	void CB_RL(uint8_t &reg);								// Rotazione a sinistra
	void CB_RR(uint8_t &reg);								// Rotazione a destra
	void CB_SLA(uint8_t &reg);								// Shift a sinistra aritmetico
	void CB_SRA(uint8_t &reg);								// Shift a destra aritmetico
	void CB_SWAP(uint8_t &reg);								// swap del nibble
	void CB_SRL(uint8_t &reg);								// Shift a destra logico
	void SBC_HL(const uint16_t *reg);						// Sottrazione a 16 bit con carry
	void LD_pnn_rr(const uint16_t *reg);					// Store in memoria da indirizzo opcode contenuto registro
	void ADC_HL(const uint16_t *reg);						// Addizzione a 16 bit con carry
	void LD_rr_pnn(uint16_t *reg);							// Load da memoria da indirizzo opcode a registro
	void NEG();												// Negazione registro A
	void LDI();												// Load and increment
	void LDIR();											// Load and increment repeat
	void LDD();												// Load and decrement
	void LDDR();											// Load and decrement repeat
	void CPI();												// Compare and increment
	void CPIR();											// Compare and increment repeat
	void CPD();												// Compare and decrement
	void CPDR();											// Compare and decrement repeat
	void LD_r_pIXOffset(uint8_t &reg);						// Legge da memoria a indirizzo IX+offset e mette in r
	void LD_pIXOffset_r(const uint8_t reg);					// Scrive in memoria a indirizzo IX+offset registo r
	void LD_pIYOffset_r(const uint8_t reg);					// Scrive in memoria a indirizzo IY+offset registo r
	void LD_pIXOffset_n();									// Scrive in memoria a indirizzo IX+offset il valore contenuto nell'opcode
	void LD_pIYOffset_n();									// Scrive in memoria a indirizzo IY+offset il valore contenuto nell'opcode
	void AND_r_pIXOffset(uint8_t &reg);						// And tra r e valore in memoria a IX+offset
	void ADD_r_pIXOffset();						// Add tra r e valore in memoria a IX+offset

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
	bool IsHalted() const { return m_halted; }

	void Interrupt();

#ifdef _DEBUG
	// Debug getters
	uint16_t GetHL() const { return HL.pair; }
	uint16_t GetDE() const { return DE.pair; }
	uint8_t GetA() const { return A; }
	uint8_t GetF() const { return F; }
	uint16_t GetBC() const { return BC.pair; }
	uint16_t GetPC() const { return PC; }
	uint8_t GetI() const { return I; }
	uint16_t GetIX() const { return IX; }
	uint16_t GetIY() const { return IY; }
	uint16_t GetSP() const { return SP; }
	uint8_t GetR() const { return R; }

	// Debug setters
	void SetHL(uint16_t value) { HL.pair = value; }
	void SetDE(uint16_t value) { DE.pair = value; }
	void SetBC(uint16_t value) { BC.pair = value; }
	void SetSP(uint16_t value) { SP = value; }
	void SetA(uint8_t value) { A = value; }
	void SetF(uint8_t value) { F = value; }

	// Memory access
	MemoryBus *GetMemory() { return m_memory; }

	// Flag access
	bool GetterFlag(uint8_t flag) const { return (F & flag) != 0; }

	uint8_t GetInterruptMode() const { return m_interruptMode; }
	bool AreInterruptsEnabled() const { return m_interruptsEnabled; }
	bool GetPendingInterrupt() const { return pendingInterrupt; }
	void SetInterruptEnabled(bool value) { m_interruptsEnabled = value; }
	uint8_t GetInterruptVector() const { return m_interruptVector; }
#endif
};