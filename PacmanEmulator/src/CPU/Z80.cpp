#include "CPU/Z80.h"
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <iomanip>

Z80::Z80(MemoryBus *memory) : m_memory(memory) {
    if (!memory) {
        throw std::invalid_argument("Memory pointer cannot be null!");
    }
    Reset();
    m_interruptMode = 1;
    m_interruptsEnabled = false;

    // inizializza register map
    m_registerMap[0] = &BC.high; // B
    m_registerMap[1] = &BC.low;  // C
    m_registerMap[2] = &DE.high; // D
    m_registerMap[3] = &DE.low;  // E
    m_registerMap[4] = &HL.high; // H
    m_registerMap[5] = &HL.low;  // L
    m_registerMap[6] = nullptr;  // (HL) - non usato direttamente
    m_registerMap[7] = &A;       // A

    // inizializza register map 16 bit
    m_register16Map[0] = &BC.pair;
    m_register16Map[1] = &DE.pair;
    m_register16Map[2] = &HL.pair;
    m_register16Map[3] = &SP;

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
    m_opcodeTable[0x86] = &Z80::OP_ADD_A_HL;
    m_opcodeTable[0x87] = &Z80::OP_ADD_A_A;

    // SUB A,r opcodes
    m_opcodeTable[0x90] = &Z80::OP_SUB_A_B;
    m_opcodeTable[0x91] = &Z80::OP_SUB_A_C;
    m_opcodeTable[0x92] = &Z80::OP_SUB_A_D;
    m_opcodeTable[0x93] = &Z80::OP_SUB_A_E;
    m_opcodeTable[0x94] = &Z80::OP_SUB_A_H;
    m_opcodeTable[0x95] = &Z80::OP_SUB_A_L;
    m_opcodeTable[0x96] = &Z80::OP_SUB_A_HL;
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
    m_opcodeTable[0xA6] = &Z80::OP_AND_A_HL;
    m_opcodeTable[0xA7] = &Z80::OP_AND_A_A;

    // XOR A,r opcodes
    m_opcodeTable[0xA8] = &Z80::OP_XOR_A_B;
    m_opcodeTable[0xA9] = &Z80::OP_XOR_A_C;
    m_opcodeTable[0xAA] = &Z80::OP_XOR_A_D;
    m_opcodeTable[0xAB] = &Z80::OP_XOR_A_E;
    m_opcodeTable[0xAC] = &Z80::OP_XOR_A_H;
    m_opcodeTable[0xAD] = &Z80::OP_XOR_A_L;
    m_opcodeTable[0xAE] = &Z80::OP_XOR_A_HL;
    m_opcodeTable[0xAF] = &Z80::OP_XOR_A_A;

    // OR A,r opcodes
    m_opcodeTable[0xB0] = &Z80::OP_OR_A_B;
    m_opcodeTable[0xB1] = &Z80::OP_OR_A_C;
    m_opcodeTable[0xB2] = &Z80::OP_OR_A_D;
    m_opcodeTable[0xB3] = &Z80::OP_OR_A_E;
    m_opcodeTable[0xB4] = &Z80::OP_OR_A_H;
    m_opcodeTable[0xB6] = &Z80::OP_OR_A_HL;
    m_opcodeTable[0xB7] = &Z80::OP_OR_A_A;

    // CP A,r opcodes
    m_opcodeTable[0xB8] = &Z80::OP_CP_A_B;
    m_opcodeTable[0xB9] = &Z80::OP_CP_A_C;
    m_opcodeTable[0xBA] = &Z80::OP_CP_A_D;
    m_opcodeTable[0xBB] = &Z80::OP_CP_A_E;
    m_opcodeTable[0xBC] = &Z80::OP_CP_A_H;
    m_opcodeTable[0xBD] = &Z80::OP_CP_A_L;
    m_opcodeTable[0xBE] = &Z80::OP_CP_A_HL;
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
    m_opcodeTable[0xE6] = &Z80::OP_AND_n;
    m_opcodeTable[0xEE] = &Z80::OP_XOR_n;
    m_opcodeTable[0xF6] = &Z80::OP_OR_n;
    m_opcodeTable[0xFE] = &Z80::OP_CP_n;
    m_opcodeTable[0x36] = &Z80::OP_LD_HL_n;

    // ADC A,r
    m_opcodeTable[0x88] = &Z80::OP_ADC_A_B;
    m_opcodeTable[0x89] = &Z80::OP_ADC_A_C;
    m_opcodeTable[0x8A] = &Z80::OP_ADC_A_D;
    m_opcodeTable[0x8B] = &Z80::OP_ADC_A_E;
    m_opcodeTable[0x8C] = &Z80::OP_ADC_A_H;
    m_opcodeTable[0x8D] = &Z80::OP_ADC_A_L;
    m_opcodeTable[0x8E] = &Z80::OP_ADC_A_HL;
    m_opcodeTable[0x8F] = &Z80::OP_ADC_A_A;
    m_opcodeTable[0xCE] = &Z80::OP_ADC_A_n;

    // SBC A,r
    m_opcodeTable[0x98] = &Z80::OP_SBC_A_B;
    m_opcodeTable[0x99] = &Z80::OP_SBC_A_C;
    m_opcodeTable[0x9A] = &Z80::OP_SBC_A_D;
    m_opcodeTable[0x9B] = &Z80::OP_SBC_A_E;
    m_opcodeTable[0x9C] = &Z80::OP_SBC_A_H;
    m_opcodeTable[0x9D] = &Z80::OP_SBC_A_L;
    m_opcodeTable[0x9E] = &Z80::OP_SBC_A_HL;
    m_opcodeTable[0x9F] = &Z80::OP_SBC_A_A;
    m_opcodeTable[0xDE] = &Z80::OP_SBC_A_n;

    // LD 16-bit
    m_opcodeTable[0x2A] = &Z80::OP_LD_HL_pnn;
    m_opcodeTable[0x22] = &Z80::OP_LD_pnn_HL;
    m_opcodeTable[0xF9] = &Z80::OP_LD_SP_HL;

    // CB
    m_opcodeTable[0xCB] = &Z80::OP_CB_Prefix;

    // ED
    m_opcodeTable[0xED] = &Z80::OP_ED_Prefix;

    // DD
    m_opcodeTable[0xDD] = &Z80::OP_DD_Prefix;

    // FD
    m_opcodeTable[0xFD] = &Z80::OP_FD_Prefix;

    // RST
    m_opcodeTable[0xC7] = &Z80::OP_RST_00;  
    m_opcodeTable[0xCF] = &Z80::OP_RST_08;  
    m_opcodeTable[0xD7] = &Z80::OP_RST_10;  
    m_opcodeTable[0xDF] = &Z80::OP_RST_18;  
    m_opcodeTable[0xE7] = &Z80::OP_RST_20;  
    m_opcodeTable[0xEF] = &Z80::OP_RST_28;  
    m_opcodeTable[0xF7] = &Z80::OP_RST_30;  
    m_opcodeTable[0xFF] = &Z80::OP_RST_38;  

    m_opcodeTable[0xEB] = &Z80::OP_EX_DE_HL;
    m_opcodeTable[0xE9] = &Z80::OP_JP_HL;

    m_opcodeTable[0xF3] = &Z80::OP_DI;
    m_opcodeTable[0x10] = &Z80::OP_DJNZ;

    m_opcodeTable[0xD3] = &Z80::OP_OUT_n_A;
    m_opcodeTable[0xFB] = &Z80::OP_EI;
    m_opcodeTable[0xFA] = &Z80::OP_JP_M_nn;
    m_opcodeTable[0x0F] = &Z80::OP_RRCA;
    m_opcodeTable[0xD9] = &Z80::OP_EXX;
    m_opcodeTable[0x2F] = &Z80::OP_CPL;
    m_opcodeTable[0x07] = &Z80::OP_RLCA;
    m_opcodeTable[0x34] = &Z80::OP_INC_pHL;
    m_opcodeTable[0x35] = &Z80::OP_DEC_pHL;
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

    std::cerr << "\nFATAL: Opcode non implementato: 0x"
        << std::hex << std::setfill('0') << std::setw(2)
        << (int)opcode << std::dec
        << " at PC=0x" << std::hex << (PC - 1) << std::dec << "\n";
    std::cerr << "Registri:\n";
    std::cerr << "  A=0x" << std::hex << (int)A << std::dec << "\n";
    std::cerr << "  BC=0x" << std::hex << BC.pair << std::dec << "\n";
    std::cerr << "  DE=0x" << std::hex << DE.pair << std::dec << "\n";
    std::cerr << "  HL=0x" << std::hex << HL.pair << std::dec << "\n";
    std::cerr << "  SP=0x" << std::hex << SP << std::dec << "\n";

    throw std::runtime_error("Unimplemented opcode encountered!");
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
void Z80::OP_ADD_A_HL()
{
    ADD_A_r(m_memory->Read(HL.pair));
    m_cyclesLastInstruction = 7;
}

// SUB A, r opcodes
void Z80::OP_SUB_A_A() { SUB_A_r(A); }
void Z80::OP_SUB_A_B() { SUB_A_r(BC.high); }
void Z80::OP_SUB_A_C() { SUB_A_r(BC.low); }
void Z80::OP_SUB_A_D() { SUB_A_r(DE.high); }
void Z80::OP_SUB_A_E() { SUB_A_r(DE.low); }
void Z80::OP_SUB_A_H() { SUB_A_r(HL.high); }
void Z80::OP_SUB_A_L() { SUB_A_r(HL.low); }
void Z80::OP_SUB_A_HL()
{
    SUB_A_r(m_memory->Read(HL.pair));
    m_cyclesLastInstruction = 7;
}

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
    m_halted = true;
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
void Z80::OP_AND_A_HL()
{
    AND_A_r(m_memory->Read(HL.pair));
    m_cyclesLastInstruction = 7;
}

// ========== XOR A,r opcodes ==========
void Z80::OP_XOR_A_A() { XOR_A_r(A); }
void Z80::OP_XOR_A_B() { XOR_A_r(BC.high); }
void Z80::OP_XOR_A_C() { XOR_A_r(BC.low); }
void Z80::OP_XOR_A_D() { XOR_A_r(DE.high); }
void Z80::OP_XOR_A_E() { XOR_A_r(DE.low); }
void Z80::OP_XOR_A_H() { XOR_A_r(HL.high); }
void Z80::OP_XOR_A_L() { XOR_A_r(HL.low); }
void Z80::OP_XOR_A_HL()
{
    XOR_A_r(m_memory->Read(HL.pair));
    m_cyclesLastInstruction = 7;
}

// ========== OR A,r opcodes ==========
void Z80::OP_OR_A_A() { OR_A_r(A); }
void Z80::OP_OR_A_B() { OR_A_r(BC.high); }
void Z80::OP_OR_A_C() { OR_A_r(BC.low); }
void Z80::OP_OR_A_D() { OR_A_r(DE.high); }
void Z80::OP_OR_A_E() { OR_A_r(DE.low); }
void Z80::OP_OR_A_H() { OR_A_r(HL.high); }
void Z80::OP_OR_A_L() { OR_A_r(HL.low); }
void Z80::OP_OR_A_HL()
{
    OR_A_r(m_memory->Read(HL.pair));
    m_cyclesLastInstruction = 7;
}

// ========== CP A,r opcodes ==========
void Z80::OP_CP_A_A() { CP_A_r(A); }
void Z80::OP_CP_A_B() { CP_A_r(BC.high); }
void Z80::OP_CP_A_C() { CP_A_r(BC.low); }
void Z80::OP_CP_A_D() { CP_A_r(DE.high); }
void Z80::OP_CP_A_E() { CP_A_r(DE.low); }
void Z80::OP_CP_A_H() { CP_A_r(HL.high); }
void Z80::OP_CP_A_L() { CP_A_r(HL.low); }
void Z80::OP_CP_A_HL()
{
    CP_A_r(m_memory->Read(HL.pair));
    m_cyclesLastInstruction = 7;
}

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

void Z80::OP_ADC_A_B() { ADC_A_r(BC.high); }
void Z80::OP_ADC_A_C() { ADC_A_r(BC.low); }
void Z80::OP_ADC_A_D() { ADC_A_r(DE.high); }
void Z80::OP_ADC_A_E() { ADC_A_r(DE.low); }
void Z80::OP_ADC_A_H() { ADC_A_r(HL.high); }
void Z80::OP_ADC_A_L() { ADC_A_r(HL.low); }
void Z80::OP_ADC_A_HL() 
{ 
    ADC_A_r(m_memory->Read(HL.pair));
    m_cyclesLastInstruction = 7; 
}
void Z80::OP_ADC_A_A() { ADC_A_r(A); }
void Z80::OP_ADC_A_n() 
{ 
    ADC_A_r(m_memory->Read(PC++));
    m_cyclesLastInstruction = 7;
}

void Z80::OP_SBC_A_B() { SBC_A_r(BC.high); }
void Z80::OP_SBC_A_C() { SBC_A_r(BC.low); }
void Z80::OP_SBC_A_D() { SBC_A_r(DE.high); }
void Z80::OP_SBC_A_E() { SBC_A_r(DE.low); }
void Z80::OP_SBC_A_H() { SBC_A_r(HL.high); }
void Z80::OP_SBC_A_L() { SBC_A_r(HL.low); }
void Z80::OP_SBC_A_HL() 
{
    SBC_A_r(m_memory->Read(HL.pair));
    m_cyclesLastInstruction = 7;
}
void Z80::OP_SBC_A_A() { SBC_A_r(A); }
void Z80::OP_SBC_A_n()
{
    SBC_A_r(m_memory->Read(PC++));
    m_cyclesLastInstruction = 7;
}

void Z80::OP_LD_HL_pnn()
{
    uint8_t add_low = m_memory->Read(PC++);
    uint8_t add_high = m_memory->Read(PC++);

    uint16_t address = ((add_high << 8) | add_low);

    uint8_t data_low = m_memory->Read(address++);
    uint8_t data_high = m_memory->Read(address);

    HL.pair = ((data_high << 8)  | data_low);

    m_cyclesLastInstruction = 16;
}

void Z80::OP_LD_pnn_HL()
{
    uint8_t add_low = m_memory->Read(PC++);
    uint8_t add_high = m_memory->Read(PC++);

    uint16_t address = ((add_high << 8) | add_low);

    m_memory->Write(address++, HL.low);
    m_memory->Write(address, HL.high);

    m_cyclesLastInstruction = 16;
}

void Z80::OP_LD_SP_HL()
{
    SP = HL.pair;

    m_cyclesLastInstruction = 6;
}

void Z80::OP_CB_Prefix()
{
    uint8_t cb_opcode = m_memory->Read(PC++);

    // decode using bit pattern
    uint8_t reg = cb_opcode & 0x07;     // bit 0-2 registro
    uint8_t operation = cb_opcode >> 3; // bit 3-7: operazione

    // Dispatch basato su operation
    if (cb_opcode < 0x40) {
        // 0x00 -0x3F: Rotate shift operation
        HandleRotateShift(operation, reg);
    }
    else if (cb_opcode < 0x80) {
        // 0x40-0x7F: Bit operations
        HandleBit(operation, reg);
    }
    else if (cb_opcode < 0xC0) {
        // 0x80-0xBF: RES operation
        HandleRes(operation, reg);
    }
    else {
        // oxC0-0xFF: Set operation
        HandleSet(operation, reg);
    }
}

void Z80::OP_ED_Prefix() 
{
    uint8_t ed_opcode = m_memory->Read(PC++);

    // IM 0 = 0xED 0x46
    if (ed_opcode == 0x46) {
        m_interruptMode = 0;
        m_cyclesLastInstruction = 8;
        return;
    }

    if (ed_opcode == 0x47) {
        // LD I, A
        I = A;
        //printf(">>> LD I, A: I = 0x%02X\n", I);
        m_cyclesLastInstruction = 9;
        return;
    }

    // IM 1 = 0xED 0x56
    if (ed_opcode == 0x56) {
        m_interruptMode = 1;
        m_cyclesLastInstruction = 8;
        return;
    }

    // IM 2 = 0xED 0x5E
    if (ed_opcode == 0x5E) {
        m_interruptMode = 2;
        //printf(">>> IM 2 set! I register will be: 0x%02X\n", I);
        m_cyclesLastInstruction = 8;
        return;
    }

    // RETI = 0xED 0x4D
    if (ed_opcode == 0x4D) {
        OP_RETI();
        return;
    }

    // Caso speciale per NEG (0x44)
    if (ed_opcode == 0x44) {
        NEG();
        return;
    }

    // Block operation
    if (ed_opcode == 0xA0) { LDI(); return; }
    if (ed_opcode == 0xB0) { LDIR(); return; }
    if (ed_opcode == 0xA8) { LDD(); return; }
    if (ed_opcode == 0xB8) { LDDR(); return; }

    // Block operations - Compare
    if (ed_opcode == 0xA1) { CPI(); return; }
    if (ed_opcode == 0xB1) { CPIR(); return; }
    if (ed_opcode == 0xA9) { CPD(); return; }
    if (ed_opcode == 0xB9) { CPDR(); return; }

    // decode using bit pattern
    uint8_t reg_index = (ed_opcode >> 4) & 0x03;     // bit 4-5 registro
    uint8_t operation = ed_opcode & 0x0F;            // bit 0-3: operazione

    // Verifica che reg_index sia valido
    if (reg_index >= 4) return;

    uint16_t *reg = m_register16Map[reg_index];

    switch (operation)
    {
    case 0x02: { SBC_HL(reg); break; }
    case 0x03: { LD_pnn_rr(reg); break; }
    case 0x0A: { ADC_HL(reg); break; }
    case 0x0B: { LD_rr_pnn(reg); break; }
    }
}

void Z80::OP_DD_Prefix()
{
    uint8_t dd_opcode = m_memory->Read(PC++);

    switch (dd_opcode) {
    case 0x21: {
        // LD IX, nn
        LD_rr_nn(IX);
        m_cyclesLastInstruction = 14;
        break;
    }
    case 0x7E: {
        // LD A, (IX+d)
        LD_r_pIXOffset(A);
        m_cyclesLastInstruction = 19;
        break;
    }
    case 0x77: {
        // LD (IX+d), A
        LD_pIXOffset_r(A);
        m_cyclesLastInstruction = 19;
        break;
    }
    case 0x34: {
        // INC (IX + d)
        int8_t offset = (int8_t)m_memory->Read(PC++);
        uint16_t address = IX + offset;
        INC_Memory(address);

        m_cyclesLastInstruction = 23;
        break;
    }
    case 0x35: {
        // DEC (IX + d)
        int8_t offset = (int8_t)m_memory->Read(PC++);
        uint16_t address = IX + offset;
        DEC_Memory(address);

        m_cyclesLastInstruction = 23;
        break;
    }
    case 0x23: {
        // INC IX
        INC_rr(IX);

        m_cyclesLastInstruction = 10;
        break;
    }
    case 0x2B: {
        // DEC IX
        DEC_rr(IX);

        m_cyclesLastInstruction = 10;
        break;
    }
    case 0x09: {
        // ADD IX,BC
        ADD_IX_rr(BC.pair);

        m_cyclesLastInstruction = 15;
        break;
    }
    case 0x19: {
        // ADD IX, DE
        ADD_IX_rr(DE.pair);

        m_cyclesLastInstruction = 15;
        break;
    }
    case 0x29: {
        // ADD IX, IX
        ADD_IX_rr(IX);

        m_cyclesLastInstruction = 15;
        break;
    }
    case 0x39: {
        // ADD IX, SP
        ADD_IX_rr(SP);

        m_cyclesLastInstruction = 15;
        break;
    }
    case 0x6E: {
        // LD L, (IX+d)
        LD_r_pIXOffset(HL.low);
        m_cyclesLastInstruction = 19;
        break;
    }
    case 0x75: {
        // LD (IX+d), L
        LD_pIXOffset_r(HL.low);
        m_cyclesLastInstruction = 19;
        break;
    }
    case 0xA6: {
        // AND A, (IX+d)
        AND_r_pIXOffset(A);
        m_cyclesLastInstruction = 19;
        break;
    }
    case 0xE5: {
        // PUSH IX
        PUSH_16bit(IX);
        m_cyclesLastInstruction = 15;
        break;
    }
    case 0xE1: {
        // POP IX
        IX = POP_16bit();
        m_cyclesLastInstruction = 14;
        break;
    }
    case 0x73: {
        // LD (IX+d), E
        LD_pIXOffset_r(DE.low);
        m_cyclesLastInstruction = 19;
        break;
    }
    case 0x70: {
        // LD (IX+d), B
        LD_pIXOffset_r(BC.high);
        m_cyclesLastInstruction = 19;
        break;
    }
    case 0x36: {
        // LD (IX+d), n
        LD_pIXOffset_n();
        m_cyclesLastInstruction = 19;
        break;
    }
    case 0x86: {
        // ADD A, (IX+d)
        ADD_r_pIXOffset();
        m_cyclesLastInstruction = 19;
        break;
    }
    case 0xCB: {
        // DD 0xCB [offset] [cb_opcode]
        int8_t offset = (int8_t)m_memory->Read(PC++);
        uint8_t cb_opcode = m_memory->Read(PC++);

        uint8_t operation = (cb_opcode >> 3) & 0x07;  // bit 3-7: operazione

        // Dispatch basato su operation (come CB normale)
        if (cb_opcode < 0x40) {
            // 0x00-0x3F: Rotate/Shift su (IX+d)
            HandleRotateShift_pIXOffset(operation, offset);
        }
        else if (cb_opcode < 0x80) {
            // 0x40-0x7F: BIT su (IX+d)
            HandleBit_pIXOffset(operation, offset);
        }
        else if (cb_opcode < 0xC0) {
            // 0x80-0xBF: RES su (IX+d)
            HandleRes_pIXOffset(operation, offset);
        }
        else {
            // 0xC0-0xFF: SET su (IX+d)
            HandleSet_pIXOffset(operation, offset);
        }
        break;
    }

    default: {
        std::cerr << "Unimplemented DD opcode: 0x" << std::hex
            << std::setfill('0') << std::setw(2)
            << (int)dd_opcode << std::dec
            << " at PC=0x" << std::hex << (PC - 1) << std::dec << "\n";
        m_cyclesLastInstruction = 4;
        break;
    }
    }
}

void Z80::OP_FD_Prefix()
{
    uint8_t fd_opcode = m_memory->Read(PC++);

    switch (fd_opcode) {
    case 0x21: {
        // LD IY, nn
        LD_rr_nn(IY);
        m_cyclesLastInstruction = 14;
        break;
    }
    case 0x7E: {
        // LD A, (IY+d)
        int8_t offset = (int8_t)m_memory->Read(PC++);
        uint16_t address = IY + offset;
        A = m_memory->Read(address);
        m_cyclesLastInstruction = 19;
        break;
    }
    case 0x77: {
        // LD (IY+d), A
        int8_t offset = (int8_t)m_memory->Read(PC++);
        uint16_t address = IY + offset;
        m_memory->Write(address, A);
        m_cyclesLastInstruction = 19;
        break;
    }
    case 0x34: {
        // INC (IY + d)
        int8_t offset = (int8_t)m_memory->Read(PC++);
        uint16_t address = IY + offset;
        INC_Memory(address);

        m_cyclesLastInstruction = 23;
        break;
    }
    case 0x35: {
        // DEC (IY + d)
        int8_t offset = (int8_t)m_memory->Read(PC++);
        uint16_t address = IY + offset;
        DEC_Memory(address);

        m_cyclesLastInstruction = 23;
        break;
    }
    case 0x36: {
        // LD (IX+d), n
        LD_pIYOffset_n();
        m_cyclesLastInstruction = 19;
        break;
    }
    case 0x23: {
        // INC IY
        INC_rr(IY);

        m_cyclesLastInstruction = 10;
        break;
    }
    case 0x2B: {
        // DEC IY
        DEC_rr(IY);

        m_cyclesLastInstruction = 10;
        break;
    }
    case 0x09: {
        // ADD IY,BC
        ADD_IY_rr(BC.pair);

        m_cyclesLastInstruction = 15;
        break;
    }
    case 0x19: {
        // ADD IY, DE
        ADD_IY_rr(DE.pair);

        m_cyclesLastInstruction = 15;
        break;
    }
    case 0x29: {
        // ADD IY, IX
        ADD_IY_rr(IY);

        m_cyclesLastInstruction = 15;
        break;
    }
    case 0x39: {
        // ADD IY, SP
        ADD_IY_rr(SP);

        m_cyclesLastInstruction = 15;
        break;
    default: {
        std::cerr << "Unimplemented FD opcode: 0x" << std::hex
            << std::setfill('0') << std::setw(2)
            << (int)fd_opcode << std::dec
            << " at PC=0x" << std::hex << (PC - 1) << std::dec << "\n";
        m_cyclesLastInstruction = 4;
        break;
    }
    }
    case 0xE5: {
        // PUSH IY
        PUSH_16bit(IY);
        m_cyclesLastInstruction = 15;
        break;
    }
    case 0x74: {
        // LD (IY+d), H
        LD_pIYOffset_r(HL.high);
        m_cyclesLastInstruction = 19;
        break;
    }
    case 0x75: {
        // LD (IY+d), L
        LD_pIYOffset_r(HL.low);
        m_cyclesLastInstruction = 19;
        break;
    }
    case 0xE1: {
        // POP IY
        IY = POP_16bit();
        m_cyclesLastInstruction = 14;
        break;
    }
    }
}

void Z80::OP_RST_00()
{
    PUSH_16bit(PC);
    PC = 0x00;
    m_cyclesLastInstruction = 11;
}

void Z80::OP_RST_08()
{
    PUSH_16bit(PC);
    PC = 0x08;
    m_cyclesLastInstruction = 11;
}

void Z80::OP_RST_10()
{
    PUSH_16bit(PC);
    PC = 0x10;
    m_cyclesLastInstruction = 11;
}

void Z80::OP_RST_18()
{
    PUSH_16bit(PC);
    PC = 0x18;
    m_cyclesLastInstruction = 11;
}

void Z80::OP_RST_20()
{
    PUSH_16bit(PC);
    PC = 0x20;
    m_cyclesLastInstruction = 11;
}

void Z80::OP_RST_28()
{
    PUSH_16bit(PC);
    PC = 0x28;
    m_cyclesLastInstruction = 11;
}

void Z80::OP_RST_30()
{
    PUSH_16bit(PC);
    PC = 0x30;
    m_cyclesLastInstruction = 11;
}

void Z80::OP_RST_38()
{
    PUSH_16bit(PC);
    PC = 0x38;
    m_cyclesLastInstruction = 11;
}

void Z80::OP_EX_DE_HL()
{
    RegisterPair temp = DE;
    DE = HL;
    HL = temp;
    m_cyclesLastInstruction = 4;
}

void Z80::OP_JP_HL()
{
    PC = HL.pair;
    m_cyclesLastInstruction = 4;
}

void Z80::OP_DI()
{
    // Disabilita interrupt - per ora ignora
    m_cyclesLastInstruction = 4;
}

void Z80::OP_DJNZ()
{
    BC.high--;  // Decrementa B

    if (BC.high != 0) {
        // Salta: leggi offset relativo e salta
        int8_t offset = (int8_t)m_memory->Read(PC++);
        PC += offset;
        m_cyclesLastInstruction = 13;  // Se salta
    }
    else {
        // Non salta: PC punta già al prossimo byte
        PC++;  // Salta l'operando offset
        m_cyclesLastInstruction = 8;   // Se non salta
    }
}

void Z80::OP_OUT_n_A()
{
    uint8_t port = m_memory->Read(PC++);

    if (port == 0x00) {
        m_interruptVector = A;
        printf(">>> OUT (0x00), A: Interrupt vector set to 0x%02X\n", A);
    }

    m_cyclesLastInstruction = 11;
}

void Z80::OP_EI()
{
    std::cout << ">>> EI opcode executed! interruptEnabled = true\n";
    m_interruptsEnabled = true;
    m_cyclesLastInstruction = 4;
}

void Z80::OP_JP_M_nn()
{
    uint8_t low = m_memory->Read(PC++);
    uint8_t high = m_memory->Read(PC++);
    uint16_t address = (high << 8) | low;

    if (GetFlag(FLAG_S)) {  // Se Sign flag è settato (negativo)
        PC = address;
    }
    // Altrimenti PC continua normalmente

    m_cyclesLastInstruction = 10;
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

void Z80::OP_RETI() 
{
    uint16_t return_addr = POP_16bit();

    PC = return_addr;
    m_interruptsEnabled = true;
    m_cyclesLastInstruction = 14;
}

void Z80::OP_RRCA() {
    // Rotate Right Circular Accumulator
    uint8_t bit0 = A & 0x01;
    A = (A >> 1) | (bit0 << 7);

    SetFlag(FLAG_C, bit0 != 0);
    SetFlag(FLAG_H, false);
    SetFlag(FLAG_N, false);
    // Z e S non vengono modificati

    m_cyclesLastInstruction = 4;
}

void Z80::OP_EXX() {
    // Scambia BC, DE, HL con BC', DE', HL'
    // (NON scambia AF/AF')

    std::swap(BC, BC_alt);
    std::swap(DE, DE_alt);
    std::swap(HL, HL_alt);

    m_cyclesLastInstruction = 4;
}

void Z80::OP_CPL()
{
    A ^= 0xFF;

    SetFlag(FLAG_H, true);
    SetFlag(FLAG_N, true);

    m_cyclesLastInstruction = 4;
}

void Z80::OP_RLCA()
{
    uint8_t bit7 = (A & 0x80) >> 7;
    A = (A << 1) | bit7;

    SetFlag(FLAG_H, false);
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_C, bit7 != 0);

    m_cyclesLastInstruction = 4;
}

void Z80::OP_INC_pHL() 
{
    INC_Memory(HL.pair);

    m_cyclesLastInstruction = 11;
}

void Z80::OP_DEC_pHL()
{
    DEC_Memory(HL.pair);

    m_cyclesLastInstruction = 11;
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

void Z80::ADD_IX_rr(uint16_t reg)
{
    uint16_t oldIX = IX;

    uint32_t result = (IX + reg);

    IX = result & 0xFFFF;

    SetFlag(FLAG_C, result > 0xFFFF);
    SetFlag(FLAG_H, ((oldIX & 0x0FFF) + (reg & 0x0FFF)) > 0x0FFF);
    SetFlag(FLAG_N, false);

    m_cyclesLastInstruction = 15;
}

void Z80::ADD_IY_rr(uint16_t reg)
{
    uint16_t oldIY = IY;

    uint32_t result = (IY + reg);

    IY = result & 0xFFFF;

    SetFlag(FLAG_C, result > 0xFFFF);
    SetFlag(FLAG_H, ((oldIY & 0x0FFF) + (reg & 0x0FFF)) > 0x0FFF);
    SetFlag(FLAG_N, false);

    m_cyclesLastInstruction = 15;
}

void Z80::INC_Memory(uint16_t address)
{
    uint8_t value = m_memory->Read(address);
    INC_r(value); // Incrementa e aggiorna flag
    m_memory->Write(address, value);
}

void Z80::DEC_Memory(uint16_t address)
{
    uint8_t value = m_memory->Read(address);
    DEC_r(value); // Incrementa e aggiorna flag
    m_memory->Write(address, value);
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

void Z80::ADC_A_r(uint8_t value)
{
    uint8_t oldA = A;
    uint8_t carry = GetFlag(FLAG_C) ? 1 : 0;
    uint16_t result = A + value + carry;

    A = result & 0xFF;

    SetFlag(FLAG_C, result > 0xFF);
    SetFlag(FLAG_Z, result == 0x00);
    SetFlag(FLAG_S, (A & 0x80) != 0);

    // Half-carry: considera anche il carry
    SetFlag(FLAG_H, ((oldA & 0x0F) + (value & 0x0F) + carry) > 0x0F);

    // Overflow: cambio di segno inaspettato
    bool overflow = ((oldA ^ result) & (value ^ result) & 0x80) != 0;
    SetFlag(FLAG_PV, overflow);

    SetFlag(FLAG_N, false);

    m_cyclesLastInstruction = 4;
}

void Z80::SBC_A_r(uint8_t value)
{
    uint8_t oldA = A;
    uint8_t carry = GetFlag(FLAG_C) ? 1 : 0;
    uint16_t result = A - value - carry;

    A = result & 0xFF;
    SetFlag(FLAG_C, oldA < (value + carry));
    SetFlag(FLAG_Z, result == 0);
    SetFlag(FLAG_S, (A & 0x80) != 0);

    // Half-carry: considera anche il carry
    SetFlag(FLAG_H, (oldA & 0x0F) < (value & 0x0F) + carry);
    
    // Overflow: cambio di segno inaspettato
    bool overflow = ((oldA ^ value) & (oldA ^ result) & 0x80) != 0;
    SetFlag(FLAG_PV, overflow);

    SetFlag(FLAG_N, true);

    m_cyclesLastInstruction = 4;
}

void Z80::HandleRotateShift(uint8_t operation, uint8_t reg)
{
    uint8_t *p_reg = m_registerMap[reg];

    // gestisci HL separatamente
    if (reg == 6) {
        uint8_t value = m_memory->Read(HL.pair);

        switch (operation) {
        case 0: CB_RLC(value); break;
        case 1: CB_RRC(value); break;
        case 2: CB_RL(value); break;
        case 3: CB_RR(value); break;
        case 4: CB_SLA(value); break;
        case 5: CB_SRA(value); break;
        case 6: CB_SWAP(value); break;
        case 7: CB_SRL(value); break;
        }
        
        m_memory->Write(HL.pair, value);

        m_cyclesLastInstruction = 15;
    }
    else {
        switch (operation) {
        case 0: CB_RLC(*p_reg); break;
        case 1: CB_RRC(*p_reg); break;
        case 2: CB_RL(*p_reg); break;
        case 3: CB_RR(*p_reg); break;
        case 4: CB_SLA(*p_reg); break;
        case 5: CB_SRA(*p_reg); break;
        case 6: CB_SWAP(*p_reg); break;
        case 7: CB_SRL(*p_reg); break;
        }

        m_cyclesLastInstruction = 8;
    }
}

void Z80::HandleBit(uint8_t bit_number, uint8_t reg)
{
    uint8_t *p_reg = m_registerMap[reg];
    if (reg == 6) {
        uint8_t value = m_memory->Read(HL.pair);
        bool bit_value = (value & (1 << bit_number)) != 0;

        SetFlag(FLAG_Z, !bit_value);
        SetFlag(FLAG_H, true);
        SetFlag(FLAG_N, false);
        SetFlag(FLAG_PV, !bit_value);  // Stesso di Z
        
        // Flag S: Solo per BIT 7, altrimenti 0
        if (bit_number == 7) {
            SetFlag(FLAG_S, bit_value);  // S = valore bit 7
        }
        else {
            SetFlag(FLAG_S, false);  // S = 0 per altri bit
        }

        m_cyclesLastInstruction = 12;
    }
    else {
        bool bit_value = (*p_reg & (1 << bit_number)) != 0;

        SetFlag(FLAG_Z, !bit_value);
        SetFlag(FLAG_H, true);
        SetFlag(FLAG_N, false);
        SetFlag(FLAG_PV, !bit_value);  // Stesso di Z
        
        // Flag S: Solo per BIT 7, altrimenti 0
        if (bit_number == 7) {
            SetFlag(FLAG_S, bit_value);  // S = valore bit 7
        }
        else {
            SetFlag(FLAG_S, false);  // S = 0 per altri bit
        }

        m_cyclesLastInstruction = 8;
    }
}

void Z80::HandleRes(uint8_t bit_number, uint8_t reg)
{
    uint8_t *p_reg = m_registerMap[reg];
    if (reg == 6) {
        uint8_t value = m_memory->Read(HL.pair);
        value &= ~(1 << bit_number); // clear bit
        m_memory->Write(HL.pair, value);

        m_cyclesLastInstruction = 15;
    }
    else {
        *p_reg &= ~(1 << bit_number); // clear bit

        m_cyclesLastInstruction = 8;
    }
}

void Z80::HandleSet(uint8_t bit_number, uint8_t reg)
{
    uint8_t *p_reg = m_registerMap[reg];
    if (reg == 6) {
        uint8_t value = m_memory->Read(HL.pair);
        value |= (1 << bit_number); // set bit
        m_memory->Write(HL.pair, value);

        m_cyclesLastInstruction = 15;
    }
    else {
        *p_reg |= (1 << bit_number); // set bit

        m_cyclesLastInstruction = 8;
    }
}

void Z80::HandleRotateShift_pIXOffset(uint8_t operation, int8_t offset)
{
    std::cerr << "HandleRotateShift_pIXOffset called: operation=0x"
        << std::hex << (int)operation << " offset=" << (int)offset << std::dec << "\n";
    // TODO: Implementare rotate/shift su (IX+d)
    m_cyclesLastInstruction = 23;
}

void Z80::HandleBit_pIXOffset(uint8_t bit_number, int8_t offset)
{
    uint16_t address = IX + offset;
    uint8_t value = m_memory->Read(address);

    bool bit_value = (value & (1 << bit_number)) != 0;

    SetFlag(FLAG_Z, !bit_value);
    SetFlag(FLAG_H, true);
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_PV, !bit_value);

    if (bit_number == 7) {
        SetFlag(FLAG_S, bit_value);
    }
    else {
        SetFlag(FLAG_S, false);
    }

    m_cyclesLastInstruction = 20;  // Operazioni memoria costano più cicli
}

void Z80::HandleRes_pIXOffset(uint8_t operation, int8_t offset)
{
    std::cerr << "HandleRes_pIXOffset called: operation=0x"
        << std::hex << (int)operation << " offset=" << (int)offset << std::dec << "\n";
    // TODO: Implementare RES su (IX+d)
    m_cyclesLastInstruction = 23;
}

void Z80::HandleSet_pIXOffset(uint8_t operation, int8_t offset)
{
    std::cerr << "HandleSet_pIXOffset called: operation=0x"
        << std::hex << (int)operation << " offset=" << (int)offset << std::dec << "\n";
    // TODO: Implementare SET su (IX+d)
    m_cyclesLastInstruction = 23;
}

void Z80::CB_RLC(uint8_t &reg)
{
    uint8_t bit7 = (reg & 0x80) >> 7;
    reg = (reg << 1) | bit7;

    SetFlag(FLAG_Z, reg == 0);
    SetFlag(FLAG_S, (reg & 0x80) != 0);
    SetFlag(FLAG_H, false);
    SetFlag(FLAG_PV, CalculateParity(reg));
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_C, bit7 != 0);
}

void Z80::CB_RRC(uint8_t &reg)
{
    uint8_t bit0 = reg & 0x01;
    reg = (bit0 << 7) | (reg >> 1);

    SetFlag(FLAG_Z, reg == 0);
    SetFlag(FLAG_S, (reg & 0x80) != 0);
    SetFlag(FLAG_H, false);
    SetFlag(FLAG_PV, CalculateParity(reg));
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_C, bit0 != 0);
}

void Z80::CB_RL(uint8_t &reg)
{
    uint8_t old_carry = GetFlag(FLAG_C) ? 1 : 0;
    uint8_t bit7 = (reg & 0x80) >> 7;
    reg = (reg << 1) | old_carry;

    SetFlag(FLAG_Z, reg == 0);
    SetFlag(FLAG_S, (reg & 0x80) != 0);
    SetFlag(FLAG_H, false);
    SetFlag(FLAG_PV, CalculateParity(reg));
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_C, bit7 != 0);
}

void Z80::CB_RR(uint8_t &reg)
{
    uint8_t old_carry = GetFlag(FLAG_C) ? 1 : 0;
    uint8_t bit0 = reg & 0x01;
    reg = (reg >> 1) | (old_carry << 7);

    SetFlag(FLAG_Z, reg == 0);
    SetFlag(FLAG_S, (reg & 0x80) != 0);
    SetFlag(FLAG_H, false);
    SetFlag(FLAG_PV, CalculateParity(reg));
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_C, bit0 != 0);
}

void Z80::CB_SLA(uint8_t &reg)
{
    uint8_t bit7 = (reg & 0x80) >> 7;
    reg <<= 1;

    SetFlag(FLAG_Z, reg == 0);
    SetFlag(FLAG_S, (reg & 0x80) != 0);
    SetFlag(FLAG_H, false);
    SetFlag(FLAG_PV, CalculateParity(reg));
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_C, bit7 != 0);
}

void Z80::CB_SRA(uint8_t &reg)
{
    uint8_t bit0 = reg & 0x01;
    uint8_t bit7 = reg & 0x80;
    reg = (reg >> 1) | bit7;

    SetFlag(FLAG_Z, reg == 0);
    SetFlag(FLAG_S, (reg & 0x80) != 0);
    SetFlag(FLAG_H, false);
    SetFlag(FLAG_PV, CalculateParity(reg));
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_C, bit0 != 0);
}

void Z80::CB_SWAP(uint8_t &reg)
{
    reg = ((reg & 0x0F) << 4) | ((reg & 0xF0) >> 4);

    SetFlag(FLAG_Z, reg == 0);
    SetFlag(FLAG_S, (reg & 0x80) != 0);
    SetFlag(FLAG_H, false);
    SetFlag(FLAG_PV, CalculateParity(reg));
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_C, false);
}

void Z80::CB_SRL(uint8_t &reg)
{
    uint8_t bit0 = reg & 0x01;
    reg = reg >> 1;

    SetFlag(FLAG_Z, reg == 0);
    SetFlag(FLAG_S, (reg & 0x80) != 0);
    SetFlag(FLAG_H, false);
    SetFlag(FLAG_PV, CalculateParity(reg));
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_C, bit0 != 0);
}

void Z80::SBC_HL(const uint16_t *reg)
{
    uint8_t oldCarry = GetFlag(FLAG_C) ? 1 : 0;
    uint16_t oldHL = HL.pair;

    int32_t result = (int32_t)HL.pair - (int32_t)*reg - oldCarry;

    HL.pair = result & 0xFFFF;

    SetFlag(FLAG_Z, HL.pair == 0x0000);
    SetFlag(FLAG_S, (HL.pair & 0x8000) != 0);
    SetFlag(FLAG_H, ((oldHL & 0x0FFF) - (*reg & 0x0FFF) - oldCarry) < 0);

    bool overflow = ((oldHL & 0x8000) != (*reg & 0x8000)) &&
                    ((oldHL & 0x8000) != (HL.pair & 0x8000));
    SetFlag(FLAG_PV, overflow);
    SetFlag(FLAG_N, true);
    SetFlag(FLAG_C, result < 0);

    m_cyclesLastInstruction = 15;
}

void Z80::LD_pnn_rr(const uint16_t *reg)
{
    // Leggi indirizzo in memoria
    uint8_t nn_low = m_memory->Read(PC++);
    uint8_t nn_high = m_memory->Read(PC++);

    // compongo l'address
    uint16_t address = nn_high << 8 | nn_low;

    // scrivo in memoria il valore contenuto nel registro
    m_memory->Write(address, *reg & 0x00FF);
    m_memory->Write(address + 1, (*reg & 0xFF00) >> 8);

    m_cyclesLastInstruction = 20;
}

void Z80::ADC_HL(const uint16_t *reg)
{
    uint8_t oldCarry = GetFlag(FLAG_C) ? 1 : 0;

    uint16_t oldHL = HL.pair;
    uint32_t result = HL.pair + *reg + oldCarry;

    HL.pair = result & 0xFFFF;

    SetFlag(FLAG_Z, HL.pair == 0x0000);
    SetFlag(FLAG_S, (HL.pair & 0x8000) != 0);
    SetFlag(FLAG_H, ((oldHL & 0x0FFF) + (*reg & 0x0FFF) + oldCarry) > 0x0FFF);

    bool overflow = ((oldHL & 0x8000) == (*reg & 0x8000)) && 
                    ((oldHL & 0x8000) != (HL.pair & 0x8000));
    SetFlag(FLAG_PV, overflow);
    SetFlag(FLAG_N, false);
    SetFlag(FLAG_C, result > 0xFFFF);

    m_cyclesLastInstruction = 15;
}

void Z80::LD_rr_pnn(uint16_t *reg)
{
    // Leggi indirizzo in memoria
    uint8_t nn_low = m_memory->Read(PC++);
    uint8_t nn_high = m_memory->Read(PC++);

    // compongo l'address
    uint16_t address = nn_high << 8 | nn_low;

    //leggo il valore di 16 bit
    uint8_t value_low = m_memory->Read(address);
    uint8_t value_high = m_memory->Read(address + 1);

    // memorizzo il valore lettonel registroù
    *reg = value_high << 8 | value_low;

    m_cyclesLastInstruction = 20;
}

void Z80::NEG()
{
    uint8_t oldA = A;
    int8_t result = 0 - (int8_t)oldA;
    A = (uint8_t)result;

    SetFlag(FLAG_Z, A == 0);
    SetFlag(FLAG_S, (A & 0x80) != 0);

    // H=1 se A aveva bit bassi (0x0F)
    bool h_borrow = (oldA & 0x0F) != 0;
    SetFlag(FLAG_H, h_borrow);

    // Overflow solo se A = 0x80 (-128)
    bool overflow = (oldA == 0x80);
    SetFlag(FLAG_PV, overflow);
    SetFlag(FLAG_N, true);

    // C = 1 sw A non era 0 (c'era un prestito)
    SetFlag(FLAG_C, oldA != 0);

    m_cyclesLastInstruction = 8;
}

void Z80::LDI()
{
    uint8_t value = m_memory->Read(HL.pair);
    m_memory->Write(DE.pair, value);
    HL.pair++;
    DE.pair++;
    BC.pair--;

    SetFlag(FLAG_H, false);
    SetFlag(FLAG_PV, BC.pair != 0);
    SetFlag(FLAG_N, false);

    m_cyclesLastInstruction = 16;
}

void Z80::LDIR()
{
    // Esegui un'interazione
    LDI();

    // Se BC != 0, ritorna al PC precedente (effetto loop)
    if (BC.pair != 0) {
        PC -= 2;
        m_cyclesLastInstruction = 21;  // Include il tentativo di jump
    }
    else {
        m_cyclesLastInstruction = 16;  // Ultima iterazione
    }
}

void Z80::LDD()
{
    uint8_t value = m_memory->Read(HL.pair);
    m_memory->Write(DE.pair, value);
    HL.pair--;
    DE.pair--;
    BC.pair--;

    SetFlag(FLAG_H, false);
    SetFlag(FLAG_PV, BC.pair != 0);
    SetFlag(FLAG_N, false);

    m_cyclesLastInstruction = 16;
}

void Z80::LDDR()
{
    // Esegui un'interazione
    LDD();

    // Se BC != 0, ritorna al PC precedente (effetto loop)
    if (BC.pair != 0) {
        PC -= 2;
        m_cyclesLastInstruction = 21;  // Include il tentativo di jump
    }
    else {
        m_cyclesLastInstruction = 16;  // Ultima iterazione
    }
}

void Z80::CPI() {
    uint8_t memValue = m_memory->Read(HL.pair);

    // Calcola il risultato della sottrazione (per i flag!)
    int16_t result = (int16_t)(int8_t)A - (int16_t)(int8_t)memValue;

    // Flag Z: Z = 1 se A == memValue
    SetFlag(FLAG_Z, A == memValue);

    // Flag S: bit di segno del risultato
    SetFlag(FLAG_S, (result & 0x80) != 0);

    // Flag H: half-borrow
    bool h_borrow = ((A & 0x0F) - (memValue & 0x0F)) < 0;
    SetFlag(FLAG_H, h_borrow);

    // Modifica HL e BC
    HL.pair++;
    BC.pair--;

    // Flag PV: (BC != 0) dopo decremento
    SetFlag(FLAG_PV, BC.pair != 0);

    // Flag N: è una sottrazione!
    SetFlag(FLAG_N, true);

    // C non viene modificato!

    m_cyclesLastInstruction = 16;
}

void Z80::CPIR() {
    CPI();

    // Ripeti se: BC != 0 AND Z == 0 (nessun match)
    if (BC.pair != 0 && !GetFlag(FLAG_Z)) {
        PC -= 2;
        m_cyclesLastInstruction = 21;
    }
    else {
        m_cyclesLastInstruction = 16;
    }
}

void Z80::CPD() {
    uint8_t memValue = m_memory->Read(HL.pair);
    int16_t result = (int16_t)(int8_t)A - (int16_t)(int8_t)memValue;

    SetFlag(FLAG_Z, A == memValue);
    SetFlag(FLAG_S, (result & 0x80) != 0);

    bool h_borrow = ((A & 0x0F) - (memValue & 0x0F)) < 0;
    SetFlag(FLAG_H, h_borrow);

    HL.pair--;  // ← Decrementa invece di incrementare!
    BC.pair--;

    SetFlag(FLAG_PV, BC.pair != 0);
    SetFlag(FLAG_N, true);

    m_cyclesLastInstruction = 16;
}

void Z80::CPDR() {
    CPD();

    // Ripeti se: BC != 0 AND Z == 0
    if (BC.pair != 0 && !GetFlag(FLAG_Z)) {
        PC -= 2;
        m_cyclesLastInstruction = 21;
    }
    else {
        m_cyclesLastInstruction = 16;
    }
}

void Z80::LD_r_pIXOffset(uint8_t &reg)
{
    int8_t offset = (int8_t)m_memory->Read(PC++);
    uint16_t address = IX + offset;
    reg = m_memory->Read(address);
}

void Z80::LD_pIXOffset_r(const uint8_t reg)
{
    int8_t offset = (int8_t)m_memory->Read(PC++);
    uint16_t address = IX + offset;
    m_memory->Write(address, reg);
}

void Z80::LD_pIYOffset_r(const uint8_t reg)
{
    int8_t offset = (int8_t)m_memory->Read(PC++);
    uint16_t address = IY + offset;
    m_memory->Write(address, reg);
}

void Z80::LD_pIXOffset_n()
{
    int8_t offset = (int8_t)m_memory->Read(PC++);
    uint8_t value = m_memory->Read(PC++);
    uint16_t address = IX + offset;
    m_memory->Write(address, value);
}

void Z80::LD_pIYOffset_n()
{
    int8_t offset = (int8_t)m_memory->Read(PC++);
    uint8_t value = m_memory->Read(PC++);
    uint16_t address = IY + offset;
    m_memory->Write(address, value);
}

void Z80::AND_r_pIXOffset(uint8_t &reg)
{
    int8_t offset = (int8_t)m_memory->Read(PC++);
    uint16_t address = IX + offset;
    uint8_t value = m_memory->Read(address);
    reg &= value;
    // Aggiorna i flag!
    SetFlag(FLAG_Z, reg == 0);
    SetFlag(FLAG_S, (reg & 0x80) != 0);
    SetFlag(FLAG_H, true);
    SetFlag(FLAG_PV, CalculateParity(reg));  // Parità
    SetFlag(FLAG_N, false);
}

void Z80::ADD_r_pIXOffset()
{
    int8_t offset = (int8_t)m_memory->Read(PC++);
    uint16_t address = IX + offset;
    uint8_t value = m_memory->Read(address);
    ADD_A_r(value);
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

void Z80::Interrupt() {
    if (!m_interruptsEnabled) return;

    m_halted = false;
    m_interruptsEnabled = false;

    PUSH_16bit(PC);

    if (m_interruptMode == 2) {
        // Usa il vettore scritto tramite OUT (0),A
        uint16_t vector_addr = (I << 8) | m_interruptVector;

        uint8_t low = m_memory->Read(vector_addr);
        uint8_t high = m_memory->Read(vector_addr + 1);
        uint16_t handler_addr = (high << 8) | low;

        /*printf(">>> Mode 2 INT: I=0x%02X, vec=0x%02X, addr=0x%04X -> handler=0x%04X\n",
            I, m_interruptVector, vector_addr, handler_addr);*/

        PC = handler_addr;
        m_totalCycles += 19;
    }
    else {
        // Mode 1: va a 0x0038
        PC = 0x0038;
        m_totalCycles += 13;
    }
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
    // Se in HALT, non eseguire nulla finché non arriva un interrupt
    if (m_halted) {
        m_cyclesLastInstruction = 4;
        m_totalCycles += 4;
        return 4; // Consuma comunque cicli
    }

    uint8_t opcode = m_memory->Read(PC++);
    //printf("DEBUG: Executing opcode 0x%02X at PC 0x%04X\n", opcode, PC - 1);
    (this->*m_opcodeTable[opcode])();
    m_totalCycles += m_cyclesLastInstruction;
    return m_cyclesLastInstruction;
}
