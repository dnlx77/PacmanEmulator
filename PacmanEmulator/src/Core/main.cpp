/*#include "Core/PacmanEmulator.h"
#include <iostream>

int main()
{
    std::cout << "=== PAC-MAN EMULATOR ===" << std::endl;
    std::cout << "Avvio emulatore..." << std::endl;

    try
    {
        PacmanEmulator emulator;

        if (!emulator.Initialize())
        {
            std::cerr << "ERRORE: Impossibile inizializzare l'emulatore!" << std::endl;
            return -1;
        }

        // TODO: Carica ROM quando sarà pronta
        // emulator.LoadROM("roms/pacman.zip");

        emulator.Run();

        std::cout << "Emulatore chiuso correttamente." << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "ECCEZIONE: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}*/

#include "Core/PacmanEmulator.h"
#include "CPU/Z80.h"
#include "Memory/MemoryBus.h"
#include <iostream>
#include <iomanip>

void PrintRegisters(Z80 &cpu) {
    std::cout << "A=0x" << std::hex << std::setw(2) << std::setfill('0') << (int)cpu.GetA()
        << " F=0x" << std::setw(2) << (int)cpu.GetF()
        << " BC=0x" << std::setw(4) << cpu.GetBC()
        << " PC=0x" << std::setw(4) << cpu.GetPC()
        << std::dec << std::endl;
}

int main() {
    try {
        MemoryBus memory;
        memory.Initialize();
        Z80 cpu(&memory);

        std::cout << "=== TEST CPU Z80 ===" << std::endl << std::endl;

        // ========== TEST 1: LD r,n ==========
        std::cout << "--- TEST 1: LD r,n (Load Immediate) ---" << std::endl;
        memory.Write(0x4800, 0x3E);  // LD A, 0x42
        memory.Write(0x4801, 0x42);
        memory.Write(0x4802, 0x06);  // LD B, 0x12
        memory.Write(0x4803, 0x12);
        memory.Write(0x4804, 0x0E);  // LD C, 0x34
        memory.Write(0x4805, 0x34);

        cpu.SetPC(0x4800);
        cpu.Step();  // LD A, 0x42
        cpu.Step();  // LD B, 0x12
        cpu.Step();  // LD C, 0x34

        std::cout << "After LD A,0x42 / LD B,0x12 / LD C,0x34:" << std::endl;
        PrintRegisters(cpu);
        std::cout << "Expected: A=0x42, BC=0x1234" << std::endl;
        std::cout << (cpu.GetA() == 0x42 && cpu.GetBC() == 0x1234 ? "PASS" : "FAIL") << std::endl << std::endl;

        // ========== TEST 2: LD r,r' ==========
        std::cout << "--- TEST 2: LD r,r' (Register Copy) ---" << std::endl;
        memory.Write(0x4806, 0x78);  // LD A, B (A = 0x12)
        memory.Write(0x4807, 0x41);  // LD B, C (B = 0x34)

        cpu.Step();  // LD A, B
        cpu.Step();  // LD B, C

        std::cout << "After LD A,B / LD B,C:" << std::endl;
        PrintRegisters(cpu);
        std::cout << "Expected: A=0x12, BC=0x3434" << std::endl;
        std::cout << (cpu.GetA() == 0x12 && cpu.GetBC() == 0x3434 ? "PASS" : "FAIL") << std::endl << std::endl;

        // ========== TEST 3: INC con Flag ==========
        std::cout << "--- TEST 3: INC A con Overflow ---" << std::endl;
        memory.Write(0x4808, 0x3E);  // LD A, 0x7F (127, max positivo)
        memory.Write(0x4809, 0x7F);
        memory.Write(0x480A, 0x3C);  // INC A (127 -> 128, overflow!)

        cpu.Step();  // LD A, 0x7F
        cpu.Step();  // INC A

        std::cout << "After LD A,0x7F / INC A:" << std::endl;
        PrintRegisters(cpu);
        std::cout << "A=0x" << std::hex << (int)cpu.GetA() << std::dec
            << " (should be 0x80)" << std::endl;
        std::cout << "Flags: Z=" << cpu.GetFlag(0x40)
            << " S=" << cpu.GetFlag(0x80)
            << " PV=" << cpu.GetFlag(0x04) << " (Overflow)" << std::endl;
        std::cout << "Expected: A=0x80, S=1, PV=1 (overflow)" << std::endl;
        std::cout << (cpu.GetA() == 0x80 && cpu.GetFlag(0x80) && cpu.GetFlag(0x04) ? "PASS" : "FAIL") << std::endl << std::endl;

        // ========== TEST 4: ADD con Carry ==========
        std::cout << "--- TEST 4: ADD A,r con Carry ---" << std::endl;
        memory.Write(0x480B, 0x3E);  // LD A, 0xFF
        memory.Write(0x480C, 0xFF);
        memory.Write(0x480D, 0x06);  // LD B, 0x02
        memory.Write(0x480E, 0x02);
        memory.Write(0x480F, 0x80);  // ADD A, B (0xFF + 0x02 = 0x101, carry!)

        cpu.Step();  // LD A, 0xFF
        cpu.Step();  // LD B, 0x02
        cpu.Step();  // ADD A, B

        std::cout << "After LD A,0xFF / LD B,0x02 / ADD A,B:" << std::endl;
        PrintRegisters(cpu);
        std::cout << "A=0x" << std::hex << (int)cpu.GetA() << std::dec
            << " (should be 0x01)" << std::endl;
        std::cout << "Flags: Z=" << cpu.GetFlag(0x40)
            << " C=" << cpu.GetFlag(0x01) << " (Carry)" << std::endl;
        std::cout << "Expected: A=0x01, C=1 (carry)" << std::endl;
        std::cout << (cpu.GetA() == 0x01 && cpu.GetFlag(0x01) ? "PASS" : "FAIL") << std::endl << std::endl;

        // ========== TEST 5: Memory Access LD A,(HL) ==========
        std::cout << "--- TEST 5: LD A,(HL) - Memory Read ---" << std::endl;
        // Prepara HL = 0x4900
        memory.Write(0x4810, 0x26);  // LD H, 0x49
        memory.Write(0x4811, 0x49);
        memory.Write(0x4812, 0x2E);  // LD L, 0x00
        memory.Write(0x4813, 0x00);
        // Scrivi valore in memoria
        memory.Write(0x4900, 0xAB);  // Valore da leggere
        memory.Write(0x4814, 0x7E);  // LD A, (HL)

        cpu.SetPC(0x4810);
        cpu.Step();  // LD H, 0x49
        cpu.Step();  // LD L, 0x00
        cpu.Step();  // LD A, (HL)

        std::cout << "After setting HL=0x4900 and LD A,(HL):" << std::endl;
        PrintRegisters(cpu);
        std::cout << "A=0x" << std::hex << (int)cpu.GetA() << std::dec
            << " (should be 0xAB)" << std::endl;
        std::cout << (cpu.GetA() == 0xAB ? "PASS" : "FAIL") << std::endl << std::endl;

        // ========== TEST 6: Memory Write LD (HL),A ==========
        std::cout << "--- TEST 6: LD (HL),r - Memory Write ---" << std::endl;
        memory.Write(0x4815, 0x3E);  // LD A, 0xCD
        memory.Write(0x4816, 0xCD);
        memory.Write(0x4817, 0x77);  // LD (HL), A

        cpu.Step();  // LD A, 0xCD
        cpu.Step();  // LD (HL), A

        uint8_t memValue = memory.Read(0x4900);
        std::cout << "After LD A,0xCD / LD (HL),A:" << std::endl;
        std::cout << "Memory[0x4900]=0x" << std::hex << (int)memValue << std::dec
            << " (should be 0xCD)" << std::endl;
        std::cout << (memValue == 0xCD ? "PASS" : "FAIL") << std::endl << std::endl;

        // ========== TEST 7: SUB con Zero Flag ==========
        std::cout << "--- TEST 7: SUB A,r con Zero Flag ---" << std::endl;
        memory.Write(0x4818, 0x3E);  // LD A, 0x10
        memory.Write(0x4819, 0x10);
        memory.Write(0x481A, 0x06);  // LD B, 0x10
        memory.Write(0x481B, 0x10);
        memory.Write(0x481C, 0x90);  // SUB A, B (0x10 - 0x10 = 0)

        cpu.Step();  // LD A, 0x10
        cpu.Step();  // LD B, 0x10
        cpu.Step();  // SUB A, B

        std::cout << "After LD A,0x10 / LD B,0x10 / SUB A,B:" << std::endl;
        PrintRegisters(cpu);
        std::cout << "Flags: Z=" << cpu.GetFlag(0x40) << " (Zero)" << std::endl;
        std::cout << "Expected: A=0x00, Z=1" << std::endl;
        std::cout << (cpu.GetA() == 0x00 && cpu.GetFlag(0x40) ? "PASS" : "FAIL") << std::endl << std::endl;

        std::cout << "=== TUTTI I TEST COMPLETATI ===" << std::endl;
        std::cout << "Total cycles executed: " << cpu.GetTotalCycles() << std::endl;

    }
    catch (const std::exception &e) {
        std::cerr << "Errore: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
