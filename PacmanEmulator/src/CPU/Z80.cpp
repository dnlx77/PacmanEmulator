#include "CPU/Z80.h"

#include <stdexcept>
#include <algorithm>

void Z80::InitOpcodeTable() {
    // Riempi tutto con "non implementato"
    for (int i = 0; i < 256; i++) {
        m_opcodeTable[i] = &Z80::OP_NotImplemented;
    }

    // NOP
    m_opcodeTable[0x00] = &Z80::OP_NOP;
    
    // INC opcodes
    m_opcodeTable[0x3C] = &Z80::OP_INC_A;
    m_opcodeTable[0x04] = &Z80::OP_INC_B;
    m_opcodeTable[0x0C] = &Z80::OP_INC_C;
    m_opcodeTable[0x14] = &Z80::OP_INC_D;
    m_opcodeTable[0x1C] = &Z80::OP_INC_E;
    m_opcodeTable[0x24] = &Z80::OP_INC_H;
    m_opcodeTable[0x2C] = &Z80::OP_INC_L;

    // DEC opcodes
    m_opcodeTable[0x3D] = &Z80::OP_DEC_A;
    m_opcodeTable[0x05] = &Z80::OP_DEC_B;
    m_opcodeTable[0x0D] = &Z80::OP_DEC_C;
    m_opcodeTable[0x15] = &Z80::OP_DEC_D;
    m_opcodeTable[0x1D] = &Z80::OP_DEC_E;
    m_opcodeTable[0x25] = &Z80::OP_DEC_H;
    m_opcodeTable[0x2D] = &Z80::OP_DEC_L;

    // LD r,n opcodes
    m_opcodeTable[0x3E] = &Z80::OP_LD_A_n;
    m_opcodeTable[0x06] = &Z80::OP_LD_B_n;
    m_opcodeTable[0x0E] = &Z80::OP_LD_C_n;
    m_opcodeTable[0x16] = &Z80::OP_LD_D_n;
    m_opcodeTable[0x1E] = &Z80::OP_LD_E_n;
    m_opcodeTable[0x26] = &Z80::OP_LD_H_n;
    m_opcodeTable[0x2E] = &Z80::OP_LD_L_n;

    // ADD A,r opcodes
    m_opcodeTable[0x87] = &Z80::OP_ADD_A_A;
    m_opcodeTable[0x80] = &Z80::OP_ADD_A_B;
    m_opcodeTable[0x81] = &Z80::OP_ADD_A_C;
    m_opcodeTable[0x82] = &Z80::OP_ADD_A_D;
    m_opcodeTable[0x83] = &Z80::OP_ADD_A_E;
    m_opcodeTable[0x84] = &Z80::OP_ADD_A_H;
    m_opcodeTable[0x85] = &Z80::OP_ADD_A_L;

    // SUB A,r opcodes
    m_opcodeTable[0x97] = &Z80::OP_SUB_A_A;
    m_opcodeTable[0x90] = &Z80::OP_SUB_A_B;
    m_opcodeTable[0x91] = &Z80::OP_SUB_A_C;
    m_opcodeTable[0x92] = &Z80::OP_SUB_A_D;
    m_opcodeTable[0x93] = &Z80::OP_SUB_A_E;
    m_opcodeTable[0x94] = &Z80::OP_SUB_A_H;
    m_opcodeTable[0x95] = &Z80::OP_SUB_A_L;

    // LD B, C
    m_opcodeTable[0x41] = &Z80::OP_LD_B_C;
}

void Z80::OP_NotImplemented() {
    // Ottieni l'opcode dell'istruzione precedente
    uint8_t opcode = m_memory->Read(PC - 1);

    // Puoi stampare un messaggio
    printf("Opcode non implementato: 0x%02X at PC=0x%04X\n", opcode, PC - 1);

    // O lanciare un'eccezione per fermare l'esecuzione
    // throw std::runtime_error("Opcode not implemented");

    m_cyclesLastInstruction = 4;  // Ciclo di default
}

void Z80::OP_NOP() {
    // Non fa nulla
    m_cyclesLastInstruction = 4;
}

// INC opcodes
void Z80::OP_INC_A() { INC_r(A); }
void Z80::OP_INC_B() { INC_r(BC.high); }
void Z80::OP_INC_C() { INC_r(BC.low); }
void Z80::OP_INC_D() { INC_r(DE.high); }
void Z80::OP_INC_E() { INC_r(DE.low); }
void Z80::OP_INC_H() { INC_r(HL.high); }
void Z80::OP_INC_L() { INC_r(HL.low); }

// DEC opcodes
void Z80::OP_DEC_A() { DEC_r(A); }
void Z80::OP_DEC_B() { DEC_r(BC.high); }
void Z80::OP_DEC_C() { DEC_r(BC.low); }
void Z80::OP_DEC_D() { DEC_r(DE.high); }
void Z80::OP_DEC_E() { DEC_r(DE.low); }
void Z80::OP_DEC_H() { DEC_r(HL.high); }
void Z80::OP_DEC_L() { DEC_r(HL.low); }

// LD R,n opcodes
void Z80::OP_LD_A_n() { LD_r_n(A); }
void Z80::OP_LD_B_n() { LD_r_n(BC.high); }
void Z80::OP_LD_C_n() { LD_r_n(BC.low); }
void Z80::OP_LD_D_n() { LD_r_n(DE.high); }
void Z80::OP_LD_E_n() { LD_r_n(DE.low); }
void Z80::OP_LD_H_n() { LD_r_n(HL.high); }
void Z80::OP_LD_L_n() { LD_r_n(HL.low); }

// ADD A, r opcodes
void Z80::OP_ADD_A_A() { ADD_A_r(A); }
void Z80::OP_ADD_A_B() { ADD_A_r(BC.high); }
void Z80::OP_ADD_A_C() { ADD_A_r(BC.low); }
void Z80::OP_ADD_A_D() { ADD_A_r(DE.high); }
void Z80::OP_ADD_A_E() { ADD_A_r(DE.low); }
void Z80::OP_ADD_A_H() { ADD_A_r(HL.high); }
void Z80::OP_ADD_A_L() { ADD_A_r(HL.low); }

// SUB A, r opcodes
void Z80::OP_SUB_A_A() { SUB_A_r(A); }
void Z80::OP_SUB_A_B() { SUB_A_r(BC.high); }
void Z80::OP_SUB_A_C() { SUB_A_r(BC.low); }
void Z80::OP_SUB_A_D() { SUB_A_r(DE.high); }
void Z80::OP_SUB_A_E() { SUB_A_r(DE.low); }
void Z80::OP_SUB_A_H() { SUB_A_r(HL.high); }
void Z80::OP_SUB_A_L() { SUB_A_r(HL.low); }

void Z80::OP_LD_B_C() { LD_r_r(BC.high, BC.low); }

void Z80::INC_r(uint8_t &reg) {
    // Salva il valore originale per calcolare i flag
    uint8_t oldValue = reg;

    // Esegui l'incremento
    reg++;

    // Calcola i flag
    SetFlag(FLAG_Z, reg == 0x00);                   // Zero
    SetFlag(FLAG_S, (reg & 0x80) != 0);             // Sign
    SetFlag(FLAG_H, (oldValue & 0x0F) == 0x0F);     // Half-Carry
    SetFlag(FLAG_PV, oldValue == 0x7F);             // Overflow
    SetFlag(FLAG_N, false);                         // N sempre 0 per addizioni
                                                    // FLAG_C non viene modificato

    m_cyclesLastInstruction = 4;
}

void Z80::DEC_r(uint8_t &reg)
{
    uint8_t oldValue = reg;
    reg--;

    SetFlag(FLAG_Z, reg == 0x00);
    SetFlag(FLAG_S, reg & 0x80);
    SetFlag(FLAG_H, (oldValue & 0x0F) == 0x00);
    SetFlag(FLAG_PV, oldValue == 0x80);
    SetFlag(FLAG_N, true);

    m_cyclesLastInstruction = 4;
}

void Z80::LD_r_n(uint8_t &reg) {
    uint8_t n = m_memory->Read(PC++);
    reg = n;
    m_cyclesLastInstruction = 7;
}

void Z80::LD_r_r(uint8_t &dest, uint8_t src) {
    dest = src;
    m_cyclesLastInstruction = 4;
}

void Z80::ADD_A_r(uint8_t value) {
    uint8_t oldA = A;
    uint16_t result = A + value;  // ? Usa 16-bit per catturare carry!

    A = result & 0xFF;

    // Flag Carry: set se risultato > 255
    SetFlag(FLAG_C, result > 0xFF);

    // Flag Zero: set se A è 0
    SetFlag(FLAG_Z, A == 0x00);

    // Flag Sign: set se bit 7 è 1
    SetFlag(FLAG_S, (A & 0x80) != 0);

    // Half-Carry: carry dal bit 3 al bit 4
    SetFlag(FLAG_H, ((oldA & 0x0F) + (value & 0x0F)) > 0x0F);

    // Overflow: cambio segno inaspettato
    // Se entrambi positivi ? risultato negativo: overflow
    // Se entrambi negativi ? risultato positivo: overflow
    bool overflow = ((oldA ^ result) & (value ^ result) & 0x80) != 0;
    SetFlag(FLAG_PV, overflow);

    // N flag: reset (è addizione)
    SetFlag(FLAG_N, false);

    m_cyclesLastInstruction = 4;
}

void Z80::SUB_A_r(uint8_t value) {
    uint8_t oldValue = A;
    uint16_t result = A - value;
    A = result & 0xFF;

    bool overflow = ((oldValue ^ value) & (oldValue ^ result) & 0x80) != 0;

    SetFlag(FLAG_C, oldValue < value);
    SetFlag(FLAG_Z, A == 0x00);
    SetFlag(FLAG_S, (A & 0x80) != 0);
    SetFlag(FLAG_H, (oldValue & 0x0F) < (value & 0x0F));
    SetFlag(FLAG_PV, overflow);
    SetFlag(FLAG_N, true);

    m_cyclesLastInstruction = 4;
}  

Z80::Z80(MemoryBus *memory) : m_memory(memory) {
	if (!memory) {
		throw std::invalid_argument("Memory pointer cannot be null!");
	}
    Reset();
    InitOpcodeTable();
}

void Z80::Reset() {
    // Program Counter
    PC = 0x0000;

    // Stack Pointer (fine della RAM principale)
    SP = 0x5000;

    // Registri general purpose
    A = 0x00;
    BC.pair = 0x0000;
    DE.pair = 0x0000;
    HL.pair = 0x0000;

    // Shadow registers
    A_alt = 0x00;
    F_alt = 0x00;
    BC_alt.pair = 0x0000;
    DE_alt.pair = 0x0000;
    HL_alt.pair = 0x0000;

    // Registri speciali
    IX = 0x0000;
    IY = 0x0000;
    I = 0x00;
    R = 0x00;

    // Flag register
    F = 0x00;

    // Cycles
    m_totalCycles = 0;
    m_cyclesLastInstruction = 0;
}

void Z80::SetFlag(uint8_t flag, bool value)
{
    if (value)
        F |= flag;  // Set bit
    else
        F &= ~flag; // clear bit
}

bool Z80::GetFlag(uint8_t flag) const
{
    return (F & flag) != 0;
}

void Z80::SetPC(uint16_t value)
{
    PC = value;
}

void Z80::ExchangeAF()
{
    std::swap(A, A_alt);
    std::swap(F, F_alt);
}

void Z80::ExchangeAll()
{
    std::swap(BC, BC_alt);
    std::swap(DE, DE_alt);
    std::swap(HL, HL_alt);
}

int Z80::Step()
{
    uint8_t opcode = m_memory->Read(PC++);
    printf("DEBUG: Executing opcode 0x%02X at PC 0x%04X\n", opcode, PC - 1);
    (this->*m_opcodeTable[opcode])();
    m_totalCycles += m_cyclesLastInstruction;
    return m_cyclesLastInstruction;
}
