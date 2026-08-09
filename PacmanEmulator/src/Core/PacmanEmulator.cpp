#include "Core/PacmanEmulator.h"
#include "Memory/MemoryBus.h"
#include <iostream>
#include <chrono>

PacmanEmulator::PacmanEmulator()
    : m_memory(nullptr), m_cpu(nullptr),
    m_videoController(nullptr),
    m_renderBackend(nullptr),
    m_isRunning(false), m_isPaused(false)
{
    std::cout << "PacmanEmulator: Costruttore chiamato" << std::endl;
}

PacmanEmulator::~PacmanEmulator()
{
    std::cout << "PacmanEmulator: Distruttore chiamato" << std::endl;
}

bool PacmanEmulator::Initialize()
{
    std::cout << "PacmanEmulator: Inizializzazione..." << std::endl;

    // Inizializza MemoryBus
    m_memory = std::make_unique<MemoryBus>();
    m_memory->Initialize();

    // Inizializza CPU Z80
    m_cpu = std::make_unique<Z80>(m_memory.get());
    m_cpu->Reset();
    
    // Inizializza il video controller
    m_videoController = std::make_unique<VideoController>(*m_memory);

    // Inizializza il render backend
    m_renderBackend = std::make_unique<SFMLBackend>();
    if (!m_renderBackend->Initialize(SCREEN_WIDTH, SCREEN_HEIGHT, 3, "Pac-Man Emulator")) {
        std::cerr << "Errore: Impossibile inizializzare il renderer" << std::endl;
        return false;
    }

    std::cout << "PacmanEmulator: Inizializzazione completata" << std::endl;
    m_isRunning = true;
    return true;
}

bool PacmanEmulator::LoadRomSet(const std::string &romDir)
{
    // Caricamento ROM
    std::cout << "PacmanEmulator: Caricamento Rom da " << romDir << std::endl;

    // Caricamento cpu roms
    for (const auto &rom : cpuRoms) {
        size_t bytesRead = m_memory->LoadRom(romDir + "/" + rom.filename, MemoryBus::ROMType::CPU, rom.offset);

        if (bytesRead!= rom.expectedSize) {
            std::cerr << "Failed: " << rom.filename << " (read " << bytesRead << " bytes, expected " << rom.expectedSize << ")\n";
            return false;
        }
    }

    // Caricamento graphics roms
    for (const auto &rom : graphicRoms) {
        size_t bytesRead = m_memory->LoadRom(romDir + "/" + rom.filename, MemoryBus::ROMType::GRAPHICS_TILES, rom.offset);

        if (bytesRead != rom.expectedSize) {
            std::cerr << "Failed: " << rom.filename << " (read " << bytesRead << " bytes, expected " << rom.expectedSize << ")\n";
            return false;
        }
    }

    // Caricamento palette rom
    size_t bytesRead = m_memory->LoadRom(romDir + "/" + graphicsPaletteFile.filename, MemoryBus::ROMType::GRAPHICS_PALETTE, graphicsPaletteFile.offset);

    if (bytesRead != graphicsPaletteFile.expectedSize) {
        std::cerr << "Failed: " << graphicsPaletteFile.filename << " (read " << bytesRead << " bytes, expected " << graphicsPaletteFile.expectedSize << ")\n";
        return false;
    }

    // Caricamento palette lookup rom
    bytesRead = m_memory->LoadRom(romDir + "/" + graphicsPaletteLookupFile.filename, MemoryBus::ROMType::PALETTE_LOOKUP, graphicsPaletteLookupFile.offset);

    if (bytesRead != graphicsPaletteLookupFile.expectedSize) {
        std::cerr << "Failed: " << graphicsPaletteLookupFile.filename << " (read " << bytesRead << " bytes, expected " << graphicsPaletteLookupFile.expectedSize << ")\n";
        return false;
    }

    std::cout << "PacmanEmulator: ROMset caricato con successo!" << std::endl;
    return true;
}

void PacmanEmulator::Run()
{
    std::cout << "PacmanEmulator: Avvio game loop..." << std::endl;

    while (m_isRunning) {
        // Gestione Input (fondamentale per chiudere o mettere in pausa)
        ProcessInput();

        if (m_isPaused) {
            sf::sleep(sf::milliseconds(10)); // Risparmia CPU se in pausa
            continue;
        }

        // --- CICLO DI SCANLINE (Rendering e CPU) ---
        // Deve arrivare fino a TOTAL_SCANLINES (288) per disegnare tutto lo schermo,
        // incluse le vite e i crediti in basso.
        for (int scanline = 0; scanline < TOTAL_SCANLINES; scanline++) {

            int cycles_this_scanline = 0;

            // Esegui la CPU per i cicli necessari a disegnare una linea (~224 cicli)
            while (cycles_this_scanline < CYCLES_PER_SCANLINE) {
                // La tua funzione Step() gestisce gi� internamente lo stato HALT
                // ritornando 4 cicli senza fare nulla, quindi possiamo chiamarla direttamente.
                cycles_this_scanline += m_cpu->Step();
            }

            // Renderizza lo sfondo (Tilemap) per questa riga
            m_videoController->RenderScanline(scanline);

            // NOTA: Abbiamo RIMOSSO il check "if (total_cycles >= FRAME)" con il break.
            // Questo garantisce che il loop arrivi fino alla riga 288.
        }

        // --- RENDERING SPRITE ---
        // Una volta disegnato tutto lo sfondo, disegniamo sopra gli sprite (Pac-Man, fantasmi).
        // (Assicurati di aver aggiunto questo metodo in VideoController come discusso prima)
        m_videoController->RenderSprites();

        // --- INTERRUPT VBLANK ---
        // Scatta una volta per frame (60Hz).
        // Controlliamo se l'hardware video lo permette (registro 0x5000)
        if (m_memory->IsIrqEnabled()) {
            m_cpu->Interrupt();
        }

        // --- AGGIORNAMENTO SCHERMO ---
        m_renderBackend->DisplayFrameBuffer(
            m_videoController->GetFrameBuffer(),
            SCREEN_WIDTH,
            SCREEN_HEIGHT
        );
        m_renderBackend->Present();
    }

    std::cout << "PacmanEmulator: Game loop terminato" << std::endl;
}

void PacmanEmulator::Reset()
{
    std::cout << "PacmanEmulator: Reset" << std::endl;

    // TODO: Reset CPU
    m_cpu->Reset();

    m_isPaused = false;
}

/*void PacmanEmulator::ProcessInput()
{
    while (std::optional<sf::Event> event = m_window->pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            m_isRunning = false;
        }

        // TODO: Gestione input tastiera per i controlli di Pac-Man
        if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>())
        {
            switch (keyPressed->code)
            {
            case sf::Keyboard::Key::Escape:
                m_isRunning = false;
                break;

            case sf::Keyboard::Key::P:
                m_isPaused = !m_isPaused;
                std::cout << "Pausa: " << (m_isPaused ? "ON" : "OFF") << std::endl;
                break;

            case sf::Keyboard::Key::R:
                Reset();
                break;

            default:
                break;
            }
        }
    }
}*/
void PacmanEmulator::ProcessInput()
{
    // Gestione eventi finestra (chiusura, ecc.)
    if (m_renderBackend->PollEvents()) {
        m_isRunning = false;
    }

    // --- GESTIONE CONTROLLI PAC-MAN (Active Low) ---
    // Default: Tutti i bit a 1 (Rilasciati)
    uint8_t in0 = 0xFF;
    uint8_t in1 = 0xFF;

    // --- PORTA IN0 (0x5000) ---
    // Bit 0: UP
    // Bit 1: LEFT
    // Bit 2: RIGHT
    // Bit 3: DOWN
    // Bit 4: RACK TEST
    // Bit 5: COIN 1
    // Bit 6: COIN 2
    // Bit 7: CREDIT BUTTON

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))    in0 &= ~0x01; // Azzera bit 0
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))  in0 &= ~0x02; // Azzera bit 1
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) in0 &= ~0x04; // Azzera bit 2
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))  in0 &= ~0x08; // Azzera bit 3

    // Tasto 5 per inserire COIN (Bit 5)
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num5))  in0 &= ~0x20;

    // --- PORTA IN1 (0x5040) ---
    // Bit 0-3: Controlli P2 (Cocktail)
    // Bit 4: SERVICE
    // Bit 5: START 1 PLAYER
    // Bit 6: START 2 PLAYERS

    // Tasto 1 per START 1P (Bit 5)
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num1))  in1 &= ~0x20;

    // Aggiorna il MemoryBus con i nuovi stati
    m_memory->UpdateInputs(in0, in1);

    // Gestione Pause/Reset (Funzioni emulatore)
    static bool p_pressed = false;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::P)) {
        if (!p_pressed) { m_isPaused = !m_isPaused; p_pressed = true; }
    }
    else { p_pressed = false; }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) m_isRunning = false;
}

void PacmanEmulator::Update(float deltaTime)
{
    // TODO: Esegui CPU cycles
    // int cyclesThisFrame = 0;
    // while (cyclesThisFrame < CPU_CYCLES_PER_FRAME)
    // {
    //     int cycles = m_cpu->Step();
    //     cyclesThisFrame += cycles;
    // }

    // TODO: Aggiorna video
    // m_video->Update();
}

void PacmanEmulator::DrawDebugInfo()
{
    // TODO: Aggiungi testo debug quando implementeremo la CPU
    // Mostra: FPS, CPU cycles, stato registri, ecc.
}