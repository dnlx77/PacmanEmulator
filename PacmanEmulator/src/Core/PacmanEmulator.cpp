#include "Core/PacmanEmulator.h"
#include "Memory/MemoryBus.h"
#include <iostream>

PacmanEmulator::PacmanEmulator()
    : m_memory(nullptr), m_cpu(nullptr),
    m_videoController(nullptr), m_window(nullptr),
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

    // Crea finestra SFML
    // Pac-Man originale: 224x288 pixel, scala x3 per visibilità
    m_window = std::make_unique<sf::RenderWindow>(
        sf::VideoMode({ 224 * 3, 288 * 3 }),
        "Pac-Man Emulator"
    );
    m_window->setFramerateLimit(60);

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
                // La tua funzione Step() gestisce già internamente lo stato HALT
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
        //m_videoController->RenderSprites();

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
    // m_cpu->Reset();

    m_isPaused = false;
}

void PacmanEmulator::ProcessInput()
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

void PacmanEmulator::Render()
{
    m_window->clear(sf::Color::Black);

    // TODO: Renderizza il gioco
    // m_video->Render(*m_window);

    // Per ora, disegna qualcosa di placeholder
    sf::CircleShape testCircle(30.f);
    testCircle.setFillColor(sf::Color::Yellow);
    testCircle.setPosition({ 300.f, 400.f });
    m_window->draw(testCircle);

    DrawDebugInfo();

    m_window->display();
}

void PacmanEmulator::DrawDebugInfo()
{
    // TODO: Aggiungi testo debug quando implementeremo la CPU
    // Mostra: FPS, CPU cycles, stato registri, ecc.
}