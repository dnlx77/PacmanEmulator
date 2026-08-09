// Harness minimale CP/M per eseguire gli esercitatori Z80 ZEXALL/ZEXDOC
// contro il nostro core Z80, per individuare bug di opcode/flag.
//
// Non e' collegato al build principale dell'emulatore Pac-Man: si compila
// e si esegue a parte, solo per validare la CPU.
//
// Uso: zexall_runner <path_al_binario.com> [indirizzo_di_carico=0x100]

#include "CPU/Z80.h"
#include "Memory/MemoryBus.h"
#include <cstdint>
#include <iostream>
#include <string>

int main(int argc, char **argv)
{
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <file.com>" << std::endl;
        return 1;
    }

    std::string path = argv[1];

    MemoryBus mem;
    mem.EnableFlatMemoryMode();

    if (!mem.LoadFlatBinary(path, 0x0100)) {
        return 1;
    }

    // Trap del "warm boot" CP/M: quando il programma ha finito salta a 0x0000.
    // HALT ci permette di accorgercene subito.
    mem.Write(0x0000, 0x76);

    // Trap della BDOS: il programma esegue "CALL 5" per stampare testo.
    // Intercettiamo la chiamata leggendo i registri PRIMA di eseguire la
    // RET che mettiamo qui, cosi' il controllo torna naturalmente al chiamante.
    mem.Write(0x0005, 0xC9);

    Z80 cpu(&mem);
    cpu.Reset();
    cpu.SetPC(0x0100);
    cpu.SetSP(0xFFFE);

    uint64_t steps = 0;
    const uint64_t MAX_STEPS = 20'000'000'000ULL; // safety net

    while (steps < MAX_STEPS) {
        uint16_t pc = cpu.GetPC();

        if (pc == 0x0000) {
            std::cout << "\n[HARNESS] Programma terminato (warm boot)." << std::endl;
            break;
        }

        if (pc == 0x0005) {
            uint16_t bc = cpu.GetBC();
            uint16_t de = cpu.GetDE();
            uint8_t function = bc & 0xFF;

            if (function == 2) {
                // C_WRITE: stampa il carattere in E
                std::cout << static_cast<char>(de & 0xFF);
            }
            else if (function == 9) {
                // C_WRITESTR: stampa la stringa terminata da '$' puntata da DE
                uint16_t addr = de;
                char ch;
                while ((ch = static_cast<char>(mem.Read(addr))) != '$') {
                    std::cout << ch;
                    addr++;
                }
            }
            std::cout.flush();
        }

        cpu.Step();
        steps++;
    }

    if (steps >= MAX_STEPS) {
        std::cout << "\n[HARNESS] Interrotto: superato il limite di sicurezza di istruzioni." << std::endl;
        return 2;
    }

    return 0;
}
