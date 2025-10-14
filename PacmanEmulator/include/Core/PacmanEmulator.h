#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <string>

// Forward declarations (le useremo dopo)
// class MemoryBus;
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
    bool LoadROM(const std::string &romPath);

    // Loop principale
    void Run();

    // Reset dell'emulatore
    void Reset();

private:
    // Componenti dell'emulatore (commentiamo per ora, li aggiungeremo dopo)
    // std::unique_ptr<MemoryBus> m_memory;
    // std::unique_ptr<Z80> m_cpu;
    // std::unique_ptr<VideoController> m_video;

    // SFML
    std::unique_ptr<sf::RenderWindow> m_window;

    // Timing
    static constexpr float FRAME_TIME = 1.0f / 60.0f; // 60 FPS
    static constexpr int CPU_CYCLES_PER_FRAME = 51200; // Z80 @ 3.072 MHz

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