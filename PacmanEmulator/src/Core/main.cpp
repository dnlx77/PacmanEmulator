#include "Core/PacmanEmulator.h"
#include "Video/VideoController.h"
#include <iostream>

/*int main() {
	PacmanEmulator emulator;

	if (!emulator.Initialize()) {
		std::cerr << "Failed to initialize\n";
		return 1;
	}

	if (!emulator.LoadRomSet("assets")) {
		std::cerr << "Failed to load romset\n";
		return 1;
	}

	VideoController video(emulator.GetMemory());
	video.RenderFrame();
	video.SaveFramebufferPPM("framebuffer.ppm");

	std::cout << "Done! Check framebuffer.ppm\n";

	emulator.Run();
	return 0;
}*/

int main() {
    PacmanEmulator emulator;
    emulator.Initialize();
    emulator.LoadRomSet("assets");

    std::cout << "=== Searching for maze-like data ===\n\n";

    // Cerca 1024 byte (tamaño della VRAM) con pattern interessante
    for (uint16_t offset = 0; offset < 0x3000; offset += 32) {
        // Leggi 1024 byte e analizza
        int unique_values = 0;
        uint8_t seen[256] = { 0 };

        for (int i = 0; i < 1024; i++) {
            uint8_t val = emulator.GetMemory().Read(offset + i);
            if (!seen[val]) {
                seen[val] = 1;
                unique_values++;
            }
        }

        // Se ha 10-50 valori unici = potrebbe essere dati, non codice
        if (unique_values >= 10 && unique_values <= 50) {
            std::cout << "Offset 0x" << std::hex << offset << std::dec
                << ": " << unique_values << " unique values\n";

            // Carica e renderizza
            for (int i = 0; i < 1024; i++) {
                uint8_t val = emulator.GetMemory().Read(offset + i);
                emulator.GetMemory().Write(0x4000 + i, val);
            }

            VideoController video(emulator.GetMemory());
            video.RenderFrame();

            std::string filename = "maze_0x" + std::to_string(offset) + ".ppm";
            video.SaveFramebufferPPM(filename);
        }
    }

    return 0;
}