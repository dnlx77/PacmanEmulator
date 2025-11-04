#include "Core/PacmanEmulator.h"
#include "Video/VideoController.h"
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>

int main() {
    PacmanEmulator emulator;
    emulator.Initialize();
    emulator.LoadRomSet("assets");

    // ... setup iniziale ...

    printf("=== Running with detailed interrupt tracing ===\n");

    const int CYCLES_PER_FRAME = 51200;
    std::map<uint16_t, int> pc_histogram;

    int frame_count = 0;
    uint16_t last_pc = 0;
    int same_pc_count = 0;

    for (int frame = 0; frame < 500; frame++) {
        int frame_cycles = 0;
        while (frame_cycles < CYCLES_PER_FRAME) {
            uint16_t pc = emulator.GetCPU()->GetPC();

            // Se PC rimane nello stesso punto, conta
            if (pc == last_pc) {
                same_pc_count++;
            }
            else {
                same_pc_count = 0;
                last_pc = pc;
            }

            // Se rimane nello STESSO PC per 10000 cicli = loop infinito!
            if (same_pc_count > 10000) {
                printf("LOOP DETECTED at PC=0x%04X after %d cycles\n", pc, same_pc_count);
                printf("Opcode: 0x%02X\n", emulator.GetMemory().Read(pc));

                // Dumpa contesto
                printf("A=0x%02X BC=0x%04X DE=0x%04X HL=0x%04X\n",
                    emulator.GetCPU()->GetA(),
                    emulator.GetCPU()->GetBC(),
                    emulator.GetCPU()->GetDE(),
                    emulator.GetCPU()->GetHL());

                return 0;  // Ferma qui
            }

            frame_cycles += emulator.GetCPU()->Step();
        }

        if (frame % 50 == 0) {
            printf("Frame %d, PC=0x%04X\n", frame, last_pc);
        }

        emulator.GetCPU()->Interrupt();
    }

    //printf("Interrupt vector written: 0x%02X\n", emulator.GetCPU()->GetInterruptVector());
    //printf("Expected vector address: 0x%04X\n",
       // (emulator.GetCPU()->GetI() << 8) | emulator.GetCPU()->GetInterruptVector());

    // Top PC addresses
    printf("\n=== Top 20 Most Executed Addresses ===\n");
    std::vector<std::pair<uint16_t, int>> sorted;
    for (auto &p : pc_histogram) {
        sorted.push_back(p);
    }
    std::sort(sorted.begin(), sorted.end(),
        [](auto &a, auto &b) { return a.second > b.second; });

    for (int i = 0; i < 20 && i < sorted.size(); i++) {
        printf("PC=0x%04X: %6d times", sorted[i].first, sorted[i].second);

        // Indica la regione
        if (sorted[i].first < 0x4000) {
            printf(" [ROM]\n");
        }
        else if (sorted[i].first >= 0x4000 && sorted[i].first < 0x4400) {
            printf(" [VRAM]\n");
        }
        else if (sorted[i].first >= 0x4400 && sorted[i].first < 0x4800) {
            printf(" [COLOR RAM]\n");
        }
        else if (sorted[i].first >= 0x4800 && sorted[i].first < 0x5000) {
            printf(" [RAM]\n");
        }
        else {
            printf(" [OTHER]\n");
        }
    }

    // Analizza il contenuto dell'interrupt handler
    printf("\n=== Interrupt Handler Disassembly (0x4073-0x4090) ===\n");
    for (int i = 0; i < 32; i++) {
        if (i % 16 == 0) printf("%04X: ", 0x4073 + i);
        printf("%02X ", emulator.GetMemory().Read(0x4073 + i));
        if (i % 16 == 15) printf("\n");
    }

    // Controlla flag/variabili di gioco
    printf("\n=== Game State Variables ===\n");
    printf("0x4E00 (credits?): 0x%02X\n", emulator.GetMemory().Read(0x4E00));
    printf("0x4E10 (mode?):    0x%02X\n", emulator.GetMemory().Read(0x4E10));
    printf("0x4E20:            0x%02X\n", emulator.GetMemory().Read(0x4E20));
    printf("0x4CC0 (semaphore):0x%02X\n", emulator.GetMemory().Read(0x4CC0));

    // VRAM dump
    printf("\n=== VRAM Content ===\n");
    for (int row = 0; row < 8; row++) {
        printf("Row %d: ", row);
        for (int col = 0; col < 28; col++) {
            printf("%02X ", emulator.GetMemory().Read(0x4000 + row * 28 + col));
        }
        printf("\n");
    }

    VideoController video(emulator.GetMemory());
    video.RenderFrame();
    video.SaveFramebufferPPM("output.ppm");

    return 0;
}