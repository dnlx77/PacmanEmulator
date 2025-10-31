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

/*void PrintRegisters(Z80 &cpu) {
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

#ifdef _DEBUG
        std::cout << "=== TEST ED-Prefix: Fase 4d - Compare Block Operations ===" << std::endl << std::endl;

        // TEST 1: CPI - No match
        std::cout << "--- TEST 1: CPI (no match) ---" << std::endl;
        cpu.SetA(0x42);
        cpu.SetHL(0x4950);
        cpu.SetBC(0x0005);

        cpu.GetMemory()->Write(0x4950, 0x99);  // A != Memory[HL]

        cpu.GetMemory()->Write(0x4810, 0xED);
        cpu.GetMemory()->Write(0x4811, 0xA1);  // CPI

        cpu.SetPC(0x4810);
        cpu.Step();

        std::cout << "After CPI (0x42 vs 0x99):" << std::endl;
        std::cout << "HL = 0x" << std::hex << cpu.GetHL() << " (expected 0x4951)" << std::endl;
        std::cout << "BC = 0x" << std::hex << cpu.GetBC() << " (expected 0x0004)" << std::endl;
        std::cout << "Z = " << (int)cpu.GetFlag(FLAG_Z) << " (expected 0 - no match)" << std::endl;
        std::cout << "PV = " << (int)cpu.GetFlag(FLAG_PV) << " (expected 1 - BC != 0)" << std::endl;
        std::cout << (cpu.GetHL() == 0x4951 && cpu.GetBC() == 0x0004 &&
            !cpu.GetFlag(FLAG_Z) && cpu.GetFlag(FLAG_PV) ? "PASS" : "FAIL")
            << std::endl << std::endl;

        // TEST 2: CPI - Match found
        std::cout << "--- TEST 2: CPI (match found) ---" << std::endl;
        cpu.SetA(0x55);
        cpu.SetHL(0x4960);
        cpu.SetBC(0x0003);

        cpu.GetMemory()->Write(0x4960, 0x55);  // A == Memory[HL]

        cpu.GetMemory()->Write(0x4812, 0xED);
        cpu.GetMemory()->Write(0x4813, 0xA1);  // CPI

        cpu.SetPC(0x4812);
        cpu.Step();

        std::cout << "After CPI (0x55 vs 0x55):" << std::endl;
        std::cout << "HL = 0x" << std::hex << cpu.GetHL() << " (expected 0x4961)" << std::endl;
        std::cout << "BC = 0x" << std::hex << cpu.GetBC() << " (expected 0x0002)" << std::endl;
        std::cout << "Z = " << (int)cpu.GetFlag(FLAG_Z) << " (expected 1 - match!)" << std::endl;
        std::cout << "PV = " << (int)cpu.GetFlag(FLAG_PV) << " (expected 1 - BC != 0)" << std::endl;
        std::cout << (cpu.GetHL() == 0x4961 && cpu.GetBC() == 0x0002 &&
            cpu.GetFlag(FLAG_Z) && cpu.GetFlag(FLAG_PV) ? "PASS" : "FAIL")
            << std::endl << std::endl;

        // TEST 3: CPIR - Find match in sequence
        std::cout << "--- TEST 3: CPIR (find match in sequence) ---" << std::endl;
        cpu.SetA(0x33);
        cpu.SetHL(0x4970);
        cpu.SetBC(0x0005);

        // Array: 0x11, 0x22, 0x33 (match!), 0x44, 0x55
        cpu.GetMemory()->Write(0x4970, 0x11);
        cpu.GetMemory()->Write(0x4971, 0x22);
        cpu.GetMemory()->Write(0x4972, 0x33);  // Match al terzo byte
        cpu.GetMemory()->Write(0x4973, 0x44);
        cpu.GetMemory()->Write(0x4974, 0x55);

        cpu.GetMemory()->Write(0x4814, 0xED);
        cpu.GetMemory()->Write(0x4815, 0xB1);  // CPIR

        cpu.SetPC(0x4814);

        // CPIR esegue finché non trova match o BC == 0
        // Iterazione 1: 0x11 != 0x33, BC = 0x0004, ripeti
        // Iterazione 2: 0x22 != 0x33, BC = 0x0003, ripeti
        // Iterazione 3: 0x33 == 0x33, Z = 1, STOP
        for (int i = 0; i < 3; i++) {
            cpu.SetPC(0x4814);
            cpu.Step();
            if (cpu.GetFlag(FLAG_Z)) break;  // Found match
        }

        std::cout << "After CPIR (searching for 0x33):" << std::endl;
        std::cout << "HL = 0x" << std::hex << cpu.GetHL() << " (expected 0x4973)" << std::endl;
        std::cout << "BC = 0x" << std::hex << cpu.GetBC() << " (expected 0x0002)" << std::endl;
        std::cout << "Z = " << (int)cpu.GetFlag(FLAG_Z) << " (expected 1 - found!)" << std::endl;
        std::cout << (cpu.GetHL() == 0x4973 && cpu.GetBC() == 0x0002 &&
            cpu.GetFlag(FLAG_Z) ? "PASS" : "FAIL")
            << std::endl << std::endl;

        // TEST 4: CPD - Decrement
        std::cout << "--- TEST 4: CPD (decrement) ---" << std::endl;
        cpu.SetA(0x77);
        cpu.SetHL(0x4974);
        cpu.SetBC(0x0002);

        cpu.GetMemory()->Write(0x4974, 0x77);

        cpu.GetMemory()->Write(0x4816, 0xED);
        cpu.GetMemory()->Write(0x4817, 0xA9);  // CPD

        cpu.SetPC(0x4816);
        cpu.Step();

        std::cout << "After CPD (0x77 vs 0x77, decrement):" << std::endl;
        std::cout << "HL = 0x" << std::hex << cpu.GetHL() << " (expected 0x4973)" << std::endl;
        std::cout << "BC = 0x" << std::hex << cpu.GetBC() << " (expected 0x0001)" << std::endl;
        std::cout << "Z = " << (int)cpu.GetFlag(FLAG_Z) << " (expected 1 - match)" << std::endl;
        std::cout << (cpu.GetHL() == 0x4973 && cpu.GetBC() == 0x0001 &&
            cpu.GetFlag(FLAG_Z) ? "PASS" : "FAIL")
            << std::endl << std::endl;

        std::cout << "=== TUTTI I TEST COMPARE BLOCK OPERATIONS COMPLETATI ===" << std::endl;
#endif

    }
    catch (const std::exception &e) {
        std::cerr << "Errore: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}*/

int main() {
    PacmanEmulator emulator;

    if (!emulator.Initialize()) {
        std::cerr << "Failed to initialize\n";
        return 1;
    }

    // ← AGGIUNGI QUESTO:
    if (!emulator.LoadRomSet("assets")) {  // Directory con i ROM
        std::cerr << "Failed to load romset\n";
        return 1;
    }

    emulator.Run();
    return 0;
}