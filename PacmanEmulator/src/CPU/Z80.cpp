#include "CPU/Z80.h"
#include <stdexcept>
#include <algorithm>

Z80::Z80(MemoryBus *memory) : m_memory(memory) {
    if (!memory) {
        throw std::invalid_argument("Memory pointer cannot be null!");
    }
    Reset();

    // inizializza register map
    m_registerMap[0] = &BC.high; // B
    m_registerMap[1] = &BC.low;  // C
    m_registerMap[2] = &DE.high; // D
    m_registerMap[3] = &DE.low;  // E
    m_registerMap[4] = &HL.high; // H
    m_registerMap[5] = &HL.low;  // L
    m_registerMap[6] = nullptr;  // (HL) - non usato direttamente
    m_registerMap[7] = &A;       // A

    InitOpcodeTable();
}

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
    m_opcodeTable[0x80] = &Z80::OP_ADD_A_B;
    m_opcodeTable[0x81] = &Z80::OP_ADD_A_C;
    m_opcodeTable[0x82] = &Z80::OP_ADD_A_D;
    m_opcodeTable[0x83] = &Z80::OP_ADD_A_E;
    m_opcodeTable[0x84] = &Z80::OP_ADD_A_H;
    m_opcodeTable[0x85] = &Z80::OP_ADD_A_L;
    m_opcodeTable[0x87] = &Z80::OP_ADD_A_A;

    // SUB A,r opcodes
    m_opcodeTable[0x90] = &Z80::OP_SUB_A_B;
    m_opcodeTable[0x91] = &Z80::OP_SUB_A_C;
    m_opcodeTable[0x92] = &Z80::OP_SUB_A_D;
    m_opcodeTable[0x93] = &Z80::OP_SUB_A_E;
    m_opcodeTable[0x94] = &Z80::OP_SUB_A_H;
    m_opcodeTable[0x95] = &Z80::OP_SUB_A_L;
    m_opcodeTable[0x97] = &Z80::OP_SUB_A_A;

    // LD r, r
    for (int i = 0x40; i <= 0x7F; i++) {
        m_opcodeTable[i] = &Z80::OP_LD_r_r;
    }

    // AND A,r opcodes
    m_opcodeTable[0xA0] = &Z80::OP_AND_A_B;
    m_opcodeTable[0xA1] = &Z80::OP_AND_A_C;
    m_opcodeTable[0xA2] = &Z80::OP_AND_A_D;
    m_opcodeTable[0xA3] = &Z80::OP_AND_A_E;
    m_opcodeTable[0xA4] = &Z80::OP_AND_A_H;
    m_opcodeTable[0xA5] = &Z80::OP_AND_A_L;
    m_opcodeTable[0xA7] = &Z80::OP_AND_A_A;

    // XOR A,r opcodes
    m_opcodeTable[0xA8] = &Z80::OP_XOR_A_B;
    m_opcodeTable[0xA9] = &Z80::OP_XOR_A_C;
    m_opcodeTable[0xAA] = &Z80::OP_XOR_A_D;
    m_opcodeTable[0xAB] = &Z80::OP_XOR_A_E;
    m_opcodeTable[0xAC] = &Z80::OP_XOR_A_H;
    m_opcodeTable[0xAD] = &Z80::OP_XOR_A_L;
    m_opcodeTable[0xAF] = &Z80::OP_XOR_A_A;

    // OR A,r opcodes
    m_opcodeTable[0xB0] = &Z80::OP_OR_A_B;
    m_opcodeTable[0xB1] = &Z80::OP_OR_A_C;
    m_opcodeTable[0xB2] = &Z80::OP_OR_A_D;
    m_opcodeTable[0xB3] = &Z80::OP_OR_A_E;
    m_opcodeTable[0xB4] = &Z80::OP_OR_A_H;
    m_opcodeTable[0xB5] = &Z80::OP_OR_A_L;
    m_opcodeTable[0xB7] = &Z80::OP_OR_A_A;

    // CP A,r opcodes
    m_opcodeTable[0xB8] = &Z80::OP_CP_A_B;
    m_opcodeTable[0xB9] = &Z80::OP_CP_A_C;
    m_opcodeTable[0xBA] = &Z80::OP_CP_A_D;
    m_opcodeTable[0xBB] = &Z80::OP_CP_A_E;
    m_opcodeTable[0xBC] = &Z80::OP_CP_A_H;
    m_opcodeTable[0xBD] = &Z80::OP_CP_A_L;
    m_opcodeTable[0xBF] = &Z80::OP_CP_A_A;

    // Jump assoluti
    m_opcodeTable[0xC3] = &Z80::OP_JP_nn;
    m_opcodeTable[0xCA] = &Z80::OP_JP_Z_nn;
    m_opcodeTable[0xC2] = &Z80::OP_JP_NZ_nn;
    m_opcodeTable[0xDA] = &Z80::OP_JP_C_nn;
    m_opcodeTable[0xD2] = &Z80::OP_JP_NC_nn;

    // Jump relativi
    m_opcodeTable[0x18] = &Z80::OP_JR_e;
    m_opcodeTable[0x28] = &Z80::OP_JR_Z_e;
    m_opcodeTable[0x20] = &Z80::OP_JR_NZ_e;
    m_opcodeTable[0x38] = &Z80::OP_JR_C_e;
    m_opcodeTable[0x30] = &Z80::OP_JR_NC_e;

    // Call/ret
    m_opcodeTable[0xCD] = &Z80::OP_CALL_nn;
    m_opcodeTable[0xCC] = &Z80::OP_CALL_Z_nn;
    m_opcodeTable[0xC4] = &Z80::OP_CALL_NZ_nn;
    m_opcodeTable[0xDC] = &Z80::OP_CALL_C_nn;
    m_opcodeTable[0xD4] = &Z80::OP_CALL_NC_nn;
    m_opcodeTable[0xC9] = &Z80::OP_RET;
    m_opcodeTable[0xC8] = &Z80::OP_RET_Z;
    m_opcodeTable[0xC0] = &Z80::OP_RET_NZ;
    m_opcodeTable[0xD8] = &Z80::OP_RET_C;
    m_opcodeTable[0xD0] = &Z80::OP_RET_NC;

    // Push/Pop
    m_opcodeTable[0xC5] = &Z80::OP_PUSH_BC;
    m_opcodeTable[0xD5] = &Z80::OP_PUSH_DE;
    m_opcodeTable[0xE5] = &Z80::OP_PUSH_HL;
    m_opcodeTable[0xF5] = &Z80::OP_PUSH_AF;
    m_opcodeTable[0xC1] = &Z80::OP_POP_BC;
    m_opcodeTable[0xD1] = &Z80::OP_POP_DE;
    m_opcodeTable[0xE1] = &Z80::OP_POP_HL;
    m_opcodeTable[0xF1] = &Z80::OP_POP_AF;

    // LD rr,nn
    m_opcodeTable[0x01] = &Z80::OP_LD_BC_nn;
    m_opcodeTable[0x11] = &Z80::OP_LD_DE_nn;
    m_opcodeTable[0x21] = &Z80::OP_LD_HL_nn;
    m_opcodeTable[0x31] = &Z80::OP_LD_SP_nn;

    // INC rr
    m_opcodeTable[0x03] = &Z80::OP_INC_BC;
    m_opcodeTable[0x13] = &Z80::OP_INC_DE;
    m_opcodeTable[0x23] = &Z80::OP_INC_HL;
    m_opcodeTable[0x33] = &Z80::OP_INC_SP;

    // DEC rr
    m_opcodeTable[0x0B] = &Z80::OP_DEC_BC;
    m_opcodeTable[0x1B] = &Z80::OP_DEC_DE;
    m_opcodeTable[0x2B] = &Z80::OP_DEC_HL;
    m_opcodeTable[0x3B] = &Z80::OP_DEC_SP;

    // ADD hL, rr
    m_opcodeTable[0x09] = &Z80::OP_ADD_HL_BC;
    m_opcodeTable[0x19] = &Z80::OP_ADD_HL_DE;
    m_opcodeTable[0x29] = &Z80::OP_ADD_HL_HL;
    m_opcodeTable[0x39] = &Z80::OP_ADD_HL_SP;

    // LD Special
    m_opcodeTable[0x0A] = &Z80::OP_LD_A_BC;
    m_opcodeTable[0x1A] = &Z80::OP_LD_A_DE;
    m_opcodeTable[0x02] = &Z80::OP_LD_BC_A;
    m_opcodeTable[0x12] = &Z80::OP_LD_DE_A;
    m_opcodeTable[0x3A] = &Z80::OP_LD_A_nn;
    m_opcodeTable[0x32] = &Z80::OP_LD_nn_A;

    // Operation Immediate
    m_opcodeTable[0xC6] = &Z80::OP_ADD_n;
    m_opcodeTable[0xD6] = &Z80::OP_SUB_n;
    m_opcodeTable[0xE6] = &Z80::OP_AND_n;;
    m_opcodeTable[0xEE] = &Z80::OP_XOR_n;;
    m_opcodeTable[0xF6] = &Z80::OP_OR_n;;
    m_opcodeTable[0xFE] = &Z80::OP_CP_n;;
    m_opcodeTable[0x36] = &Z80::OP_LD_HL_n;
}

bool Z80::CalculateParity(uint8_t value)
{
    int count = 0;
    while (value) {
        if (value & 0x01) {
            count++;
        }
        value >>= 1;
    }
    return (count % 2) == 0;
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

void Z80::OP_LD_r_r()
{
    uint8_t opcode = m_memory->Read(PC - 1);
    if (opcode == 0x76) {
        OP_HALT();
        return;
    }

    uint8_t dest = (opcode & 0x38) >> 3;
    uint8_t src = opcode & 0x07;

    if (src == 6) {
        // Lettura dalla memoria
        *m_registerMap[dest] = m_memory->Read(HL.pair);
        m_cyclesLastInstruction = 7;
    }
    else if (dest == 6) {
        // Scrittura in memoria
        m_memory->Write(HL.pair, *m_registerMap[src]);
        m_cyclesLastInstruction = 7;
    }
    else {
        // Entrambi registri normali
        *m_registerMap[dest] = *m_registerMap[src];
        m_cyclesLastInstruction = 4;
    }
  }

void Z80::OP_HALT()
{
    printf("Esecuzione opcode HALT");
    m_cyclesLastInstruction = 4;
}

void Z80::OP_JP_nn()
{
    // Leggi dalla memoria l'indirizzo della call
    uint8_t low = m_memory->Read(PC++);
    uint8_t high = m_memory->Read(PC++);

    PC = (high << 8) | low;

    m_cyclesLastInstruction = 10;
}

void Z80::OP_JP_Z_nn() { JP_nn_conditional(FLAG_Z, true); }

void Z80::OP_JP_NZ_nn() { JP_nn_conditional(FLAG_Z, false); }

void Z80::OP_JP_C_nn() { JP_nn_conditional(FLAG_C, true); }

void Z80::OP_JP_NC_nn() { JP_nn_conditional(FLAG_C, false); }

void Z80::OP_JR_e()
{
    uint8_t offset_unsigned = m_memory->Read(PC++);
    int8_t offset = static_cast<int8_t>(offset_unsigned);

    PC += offset;

    m_cyclesLastInstruction = 12;
}

void Z80::OP_JR_Z_e() { JR_conditional(FLAG_Z, true); }

void Z80::OP_JR_NZ_e() { JR_conditional(FLAG_Z, false); }

void Z80::OP_JR_C_e() { JR_conditional(FLAG_C, true); }

void Z80::OP_JR_NC_e() { JR_conditional(FLAG_C, false); }

// ========== AND A,r opcodes ==========
void Z80::OP_AND_A_A() { AND_A_r(A); }
void Z80::OP_AND_A_B() { AND_A_r(BC.high); }
void Z80::OP_AND_A_C() { AND_A_r(BC.low); }
void Z80::OP_AND_A_D() { AND_A_r(DE.high); }
void Z80::OP_AND_A_E() { AND_A_r(DE.low); }
void Z80::OP_AND_A_H() { AND_A_r(HL.high); }
void Z80::OP_AND_A_L() { AND_A_r(HL.low); }

// ========== XOR A,r opcodes ==========
void Z80::OP_XOR_A_A() { XOR_A_r(A); }
void Z80::OP_XOR_A_B() { XOR_A_r(BC.high); }
void Z80::OP_XOR_A_C() { XOR_A_r(BC.low); }
void Z80::OP_XOR_A_D() { XOR_A_r(DE.high); }
void Z80::OP_XOR_A_E() { XOR_A_r(DE.low); }
void Z80::OP_XOR_A_H() { XOR_A_r(HL.high); }
void Z80::OP_XOR_A_L() { XOR_A_r(HL.low); }

// ========== OR A,r opcodes ==========
void Z80::OP_OR_A_A() { OR_A_r(A); }
void Z80::OP_OR_A_B() { OR_A_r(BC.high); }
void Z80::OP_OR_A_C() { OR_A_r(BC.low); }
void Z80::OP_OR_A_D() { OR_A_r(DE.high); }
void Z80::OP_OR_A_E() { OR_A_r(DE.low); }
void Z80::OP_OR_A_H() { OR_A_r(HL.high); }
void Z80::OP_OR_A_L() { OR_A_r(HL.low); }

// ========== CP A,r opcodes ==========
void Z80::OP_CP_A_A() { CP_A_r(A); }
void Z80::OP_CP_A_B() { CP_A_r(BC.high); }
void Z80::OP_CP_A_C() { CP_A_r(BC.low); }
void Z80::OP_CP_A_D() { CP_A_r(DE.high); }
void Z80::OP_CP_A_E() { CP_A_r(DE.low); }
void Z80::OP_CP_A_H() { CP_A_r(HL.high); }
void Z80::OP_CP_A_L() { CP_A_r(HL.low); }

void Z80::OP_CALL_nn()
{
    // Leggi dalla memoria l'indirizzo della call
    uint8_t low = m_memory->Read(PC++);
    uint8_t high = m_memory->Read(PC++);

    // Salvo sullo stack il valore attuale di PC
    PUSH_16bit(PC);

    //Imposto PC con l'istruzione da eseguire
    PC = (high << 8) | low;

    m_cyclesLastInstruction = 17;
}

void Z80::OP_CALL_Z_nn() { CALL_conditional(FLAG_Z, true); }

void Z80::OP_CALL_NZ_nn() { CALL_conditional(FLAG_Z, false); }

void Z80::OP_CALL_C_nn() { CALL_conditional(FLAG_C, true); }

void Z80::OP_CALL_NC_nn() { CALL_conditional(FLAG_C, false); }

void Z80::OP_RET()
{
    PC = POP_16bit();

    m_cyclesLastInstruction = 10;
}

void Z80::OP_RET_Z() { RET_conditional(FLAG_Z, true); }

void Z80::OP_RET_NZ() { RET_conditional(FLAG_Z, false); }

void Z80::OP_RET_C() { RET_conditional(FLAG_C, true); }

void Z80::OP_RET_NC() { RET_conditional(FLAG_C, false); }

void Z80::OP_PUSH_BC() { PUSH_rr(&BC); }

void Z80::OP_PUSH_DE() { PUSH_rr(&DE); }

void Z80::OP_PUSH_HL() { PUSH_rr(&HL); }

void Z80::OP_PUSH_AF()
{
    uint16_t af = (A << 8) | F;
    PUSH_16bit(af);

    m_cyclesLastInstruction = 11;
}

void Z80::OP_POP_BC() { POP_rr(&BC); }

void Z80::OP_POP_DE() { POP_rr(&DE); }

void Z80::OP_POP_HL() { POP_rr(&HL); }

void Z80::OP_POP_AF()
{
    uint16_t af = POP_16bit();
    
    A = (af & 0xFF00) >> 8;
    F = af & 0xFF;

    m_cyclesLastInstruction = 10;
}

void Z80::OP_LD_BC_nn() { LD_rr_nn(BC.pair); }

void Z80::OP_LD_DE_nn() { LD_rr_nn(DE.pair); }

void Z80::OP_LD_HL_nn() { LD_rr_nn(HL.pair); }

void Z80::OP_LD_SP_nn() { LD_rr_nn(SP); }

void Z80::OP_INC_BC() { INC_rr(BC.pair); }

void Z80::OP_INC_DE() { INC_rr(DE.pair); }

void Z80::OP_INC_HL() { INC_rr(HL.pair); }

void Z80::OP_INC_SP() { INC_rr(SP); }

void Z80::OP_DEC_BC() { DEC_rr(BC.pair); }

void Z80::OP_DEC_DE() { DEC_rr(DE.pair); }

void Z80::OP_DEC_HL() { DEC_rr(HL.pair); }

void Z80::OP_DEC_SP() { DEC_rr(SP); }

void Z80::OP_ADD_HL_BC() { ADD_HL_rr(BC.pair); }

void Z80::OP_ADD_HL_DE() { ADD_HL_rr(DE.pair); }

void Z80::OP_ADD_HL_HL() { ADD_HL_rr(HL.pair); }

void Z80::OP_ADD_HL_SP() { ADD_HL_rr(SP); }

void Z80::OP_LD_A_BC() { LD_A_indirect(BC.pair); }

void Z80::OP_LD_A_DE() { LD_A_indirect(DE.pair); } 

void Z80::OP_LD_BC_A() { LD_indirect_A(BC.pair); }

void Z80::OP_LD_DE_A() { LD_indirect_A(DE.pair); }

void Z80::OP_LD_A_nn() { LD_A_addr(); }

void Z80::OP_LD_nn_A() { LD_addr_A(); }

void Z80::OP_ADD_n()
{
    uint8_t n = m_memory->Read(PC++);
    ADD_A_r(n);
    m_cyclesLastInstruction = 7;
}

void Z80::OP_SUB_n()
{
    uint8_t n = m_memory->Read(PC++);
    SUB_A_r(n);
    m_cyclesLastInstruction = 7;
}

void Z80::OP_AND_n()
{
    uint8_t n = m_memory->Read(PC++);
    AND_A_r(n);
    m_cyclesLastInstruction = 7;
}

void Z80::OP_XOR_n()
{
    uint8_t n = m_memory->Read(PC++);
    XOR_A_r(n);
    m_cyclesLastInstruction = 7;
}

void Z80::OP_OR_n()
{
    uint8_t n = m_memory->Read(PC++);
    OR_A_r(n);
    m_cyclesLastInstruction = 7;
}

void Z80::OP_CP_n()
{
    uint8_t n = m_memory->Read(PC++);
    CP_A_r(n);
    m_cyclesLastInstruction = 7;
}

void Z80::OP_LD_HL_n()
{
    uint8_t n = m_memory->Read(PC++);
    m_memory->Write(HL.pair, n);

    m_cyclesLastInstruction = 12;
}

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

    // Flag Zero: set se A � 0
    SetFlag(FLAG_Z, A == 0x00);

    // Flag Sign: set se bit 7 � 1
    SetFlag(FLAG_S, (A & 0x80) != 0);

    // Half-Carry: carry dal bit 3 al bit 4
    SetFlag(FLAG_H, ((oldA & 0x0F) + (value & 0x0F)) > 0x0F);

    // Overflow: cambio segno inaspettato
    // Se entrambi positivi ? risultato negativo: overflow
    // Se entrambi negativi ? risultato positivo: overflow
    bool overflow = ((oldA ^ result) & (value ^ result) & 0x80) != 0;
    SetFlag(FLAG_PV, overflow);

    // N flag: reset (� addizione)
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

void Z80::AND_A_r(uint8_t value)
{
    A &= value;

    SetFlag(FLAG_Z, A == 0x00);
    SetFlag(FLAG_S, (A & 0x80) != 0);
    SetFlag(FLAG_PV, CalculateParity(A));
    SetFlag(FLAG_H, true);
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_C, false);

    m_cyclesLastInstruction = 4;
}

void Z80::OR_A_r(uint8_t value)
{
    A |= value;

    SetFlag(FLAG_Z, A == 0x00);
    SetFlag(FLAG_S, (A & 0x80) != 0);
    SetFlag(FLAG_PV, CalculateParity(A));
    SetFlag(FLAG_H, false);
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_C, false);

    m_cyclesLastInstruction = 4;
}

void Z80::XOR_A_r(uint8_t value)
{
    A ^= value;

    SetFlag(FLAG_Z, A == 0x00);
    SetFlag(FLAG_S, (A & 0x80) != 0);
    SetFlag(FLAG_PV, CalculateParity(A));
    SetFlag(FLAG_H, false);
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_C, false);
    
    m_cyclesLastInstruction = 4;
}

void Z80::CP_A_r(uint8_t value)
{
    uint8_t oldA = A;
    SUB_A_r(value);
    A = oldA;

    m_cyclesLastInstruction = 4;
}

void Z80::RET_conditional(uint8_t flag, bool condition)
{
    bool flagValue = GetFlag(flag);

    if (flagValue == condition) {
        PC = POP_16bit();
        m_cyclesLastInstruction = 11;
    }
    else {
        m_cyclesLastInstruction = 5;
    }
}

void Z80::CALL_conditional(uint8_t flag, bool condition)
{
    bool flagValue = GetFlag(flag);

    // Leggi dalla memoria l'indirizzo della call
    uint8_t low = m_memory->Read(PC++);
    uint8_t high = m_memory->Read(PC++);
    
    if (flagValue == condition) {
        // Salvo sullo stack il valore attuale di PC
        PUSH_16bit(PC);

        //Imposto PC con l'istruzione da eseguire
        PC = (high << 8) | low;

        m_cyclesLastInstruction = 17;
    }
    else {
        m_cyclesLastInstruction = 10;
    }
}

void Z80::JP_nn_conditional(uint8_t flag, bool condition)
{
    bool flagValue = GetFlag(flag);

    // Leggi dalla memoria l'indirizzo della call
    uint8_t low = m_memory->Read(PC++);
    uint8_t high = m_memory->Read(PC++);

    if (flagValue == condition) {
        PC = (high << 8) | low;
    }

    m_cyclesLastInstruction = 10;
}

void Z80::JR_conditional(uint8_t flag, bool condition)
{
    bool flagValue = GetFlag(flag);

    uint8_t offset_unsigned = m_memory->Read(PC++);
    int8_t offset = static_cast<int8_t>(offset_unsigned);

    if (flagValue == condition) {
        PC += offset;
        m_cyclesLastInstruction = 12;
    }
    else {
        m_cyclesLastInstruction = 7;
    }
}

void Z80::PUSH_rr(RegisterPair const *reg)
{
    PUSH_16bit(reg->pair);

    m_cyclesLastInstruction = 11;
}

void Z80::POP_rr(RegisterPair *reg)
{
    reg->pair = POP_16bit();

    m_cyclesLastInstruction = 10;
}

void Z80::LD_rr_nn(uint16_t &reg)
{
    uint8_t low = m_memory->Read(PC++);
    uint8_t high = m_memory->Read(PC++);
    
    reg = (high << 8) | low;

    m_cyclesLastInstruction = 10;
}

void Z80::INC_rr(uint16_t &reg)
{
    reg++;

    m_cyclesLastInstruction = 6;
}

void Z80::DEC_rr(uint16_t &reg)
{
    reg--;

    m_cyclesLastInstruction = 6;
}

void Z80::ADD_HL_rr(uint16_t reg)
{
    uint16_t oldHL = HL.pair;

    uint32_t result = (HL.pair + reg);

    HL.pair = result & 0xFFFF;

    SetFlag(FLAG_C, result > 0xFFFF);
    SetFlag(FLAG_H, ((oldHL & 0x0FFF) + (reg & 0x0FFF)) > 0x0FFF);
    SetFlag(FLAG_N, false);

    m_cyclesLastInstruction = 11;
}

void Z80::LD_A_indirect(uint16_t address)
{
    A = m_memory->Read(address);

    m_cyclesLastInstruction = 7;
}

void Z80::LD_indirect_A(uint16_t address)
{
    m_memory->Write(address, A);

    m_cyclesLastInstruction = 7;
}

void Z80::LD_A_addr()
{
    uint8_t low = m_memory->Read(PC++);
    uint8_t high = m_memory->Read(PC++);

    A = m_memory->Read((high << 8) | low);

    m_cyclesLastInstruction = 10;
}

void Z80::LD_addr_A()
{
    uint8_t low = m_memory->Read(PC++);
    uint8_t high = m_memory->Read(PC++);

    m_memory->Write((high << 8) | low, A);

    m_cyclesLastInstruction = 13;
}

void Z80::PUSH_16bit(uint16_t value)
{
    uint8_t low = value & 0xFF;
    uint8_t high = (value >> 8) & 0xFF;
    m_memory->Write(--SP, high);
    m_memory->Write(--SP, low);

}

uint16_t Z80::POP_16bit()
{
    uint8_t low = m_memory->Read(SP++);
    uint8_t high = m_memory->Read(SP++);
    uint16_t address = (high << 8) | low;
    return address;
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
