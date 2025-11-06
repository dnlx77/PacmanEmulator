#pragma once

#include <SFML/Graphics.hpp>
#include <CPU/Z80.h>
#include <memory>
#include <string>
#include "Video/VideoController.h"
#include "Video/SFMLBackend.h"

class MemoryBus;
// // Forward declarations (le useremo dopo)
// class Z80;
// class VideoController;

class PacmanEmulator
{
public:
    PacmanEmulator();
    ~PacmanEmulator();

    // Previeni copia
    PacmanEmulator(const PacmanEmulator &) = delete;
    PacmanEmulator &operator=(const PacmanEmulator &) = delete;

    // Inizializza l'emulatore
    bool Initialize();

    // Carica le ROM di Pac-Man
    bool LoadRomSet(const std::string &romDir);

    // Loop principale
    void Run();

    // Reset dell'emulatore
    void Reset();

    MemoryBus &GetMemory() const { return *m_memory; }
    Z80 *GetCPU() { return m_cpu.get(); }

private:
    std::unique_ptr<MemoryBus> m_memory;
    // Componenti dell'emulatore (commentiamo per ora, li aggiungeremo dopo)
    std::unique_ptr<Z80> m_cpu;
    std::unique_ptr<VideoController> m_videoController;

    // SFML
    std::unique_ptr<sf::RenderWindow> m_window;

    // Render backend
    std::unique_ptr<RenderBackend> m_renderBackend;

    // Timing
    static constexpr int Z80_FREQUENCY = 3072000;  // 3.072 MHz
    static constexpr int CYCLES_PER_FRAME = Z80_FREQUENCY / 60;  // ~51.200
    static constexpr int CYCLES_PER_SCANLINE = 224;
    static constexpr int TOTAL_SCANLINES = 288;

    // Stato
    bool m_isRunning;
    bool m_isPaused;

    // Metodi privati
    void ProcessInput();
    void Update(float deltaTime);
    void Render();

    // Debug
    void DrawDebugInfo();
};