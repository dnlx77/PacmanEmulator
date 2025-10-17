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

int main() {
    try {
        MemoryBus memory;
        memory.Initialize();
        Z80 cpu(&memory);

        std::cout << "=== Test CPU Z80 ===" << std::endl;

        // Scrivi nella RAM (0x4800-0x4FFF)
        memory.Write(0x4800, 0x00);        // NOP
        memory.Write(0x4801, 0x3E);        // LD A, n
        memory.Write(0x4802, 0x42);        // operando
        memory.Write(0x4803, 0x3C);        // INC A
        memory.Write(0x4804, 0x41);        // LD B, C

        // Imposta PC alla RAM
        cpu.SetPC(0x4800);

        std::cout << "PC iniziale: 0x" << std::hex << cpu.GetPC() << std::endl;

        // Esegui le istruzioni
        int cycles = cpu.Step();
        std::cout << "1. NOP - Cycles: " << std::dec << cycles
            << " | PC: 0x" << std::hex << cpu.GetPC() << std::endl;

        cycles = cpu.Step();
        std::cout << "2. LD A, 0x42 - Cycles: " << std::dec << cycles
            << " | A: 0x" << std::hex << (int)cpu.GetA()
            << " | PC: 0x" << cpu.GetPC() << std::endl;

        cycles = cpu.Step();
        std::cout << "3. INC A - Cycles: " << std::dec << cycles
            << " | A: 0x" << std::hex << (int)cpu.GetA()
            << " | F: 0x" << (int)cpu.GetF()
            << " | PC: 0x" << cpu.GetPC() << std::endl;

        cycles = cpu.Step();
        std::cout << "4. LD B, C - Cycles: " << std::dec << cycles
            << " | BC: 0x" << std::hex << cpu.GetBC()
            << " | PC: 0x" << cpu.GetPC() << std::endl;

        std::cout << "\nTest completato con successo!" << std::endl;

    }
    catch (const std::exception &e) {
        std::cerr << "Errore: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}