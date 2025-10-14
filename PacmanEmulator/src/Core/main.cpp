#include "Core/PacmanEmulator.h"
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
}