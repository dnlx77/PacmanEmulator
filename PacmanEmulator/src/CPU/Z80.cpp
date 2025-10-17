#include "CPU/Z80.h"

#include <stdexcept>
#include <algorithm>

void Z80::InitOpcodeTable() {
    // Riempi tutto con "non implementato"
    for (int i = 0; i < 256; i++) {
        m_opcodeTable[i] = &Z80::OP_NotImplemented;
    }

    // Sovrascrivi quelli implementati
    m_opcodeTable[0x00] = &Z80::OP_NOP;
    m_opcodeTable[0x3C] = &Z80::OP_INC_A;
    m_opcodeTable[0x3E] = &Z80::OP_LD_A_n;
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

void Z80::OP_INC_A() {
    // Salva il valore originale per calcolare i flag
    uint8_t oldA = A;

    // Esegui l'incremento
    A++;

    // Calcola i flag
    SetFlag(FLAG_Z, A == 0x00);                    // Zero
    SetFlag(FLAG_S, (A & 0x80) != 0);              // Sign
    SetFlag(FLAG_H, (oldA & 0x0F) == 0x0F);        // Half-Carry
    SetFlag(FLAG_PV, oldA == 0x7F);                // Overflow
    SetFlag(FLAG_N, false);                         // N sempre 0 per addizioni
    // FLAG_C non viene modificato

    m_cyclesLastInstruction = 4;
}

void Z80::OP_LD_A_n() {
    uint8_t n = m_memory->Read(PC++);
    A = n;
    m_cyclesLastInstruction = 7;
}

void Z80::OP_LD_B_C() {
    BC.high = BC.low;
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
