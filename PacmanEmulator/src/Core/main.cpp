#include "Core/PacmanEmulator.h"
#include <iostream>

int main()
{
    try {
        std::cout << "=== PAC-MAN EMULATOR ===" << std::endl;
        
        // 1. Crea l'emulatore
        PacmanEmulator emulator;
        
        // 2. Inizializza
        if (!emulator.Initialize()) {
            std::cerr << "Errore: impossibile inizializzare l'emulatore" << std::endl;
            return -1;
        }
        
        // 3. Carica la ROM
        if (!emulator.LoadRomSet("assets")) {
            std::cerr << "Errore: impossibile caricare la ROM" << std::endl;
            return -1;
        }
        
        // 4. Avvia il game loop
        emulator.Run();
        
        std::cout << "Emulatore terminato correttamente" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Eccezione: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}