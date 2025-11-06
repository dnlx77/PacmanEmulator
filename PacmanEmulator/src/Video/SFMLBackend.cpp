#include "Video/SFMLBackend.h"
#include <iostream>

SFMLBackend::SFMLBackend()
    : m_window(nullptr), m_frameTexture(nullptr),
    m_frameSprite(nullptr), m_width(0), m_height(0), m_scale(1)
{
}

SFMLBackend::~SFMLBackend()
{
    Shutdown();
}

bool SFMLBackend::Initialize(unsigned int width, unsigned int height, unsigned int scale,
    const std::string &title)
{
    // 1. Salva i parametri
    m_width = width;
    m_height = height;
    m_scale = scale;
    m_win_title = title;

    // 2. Crea la finestra SFML con dimensioni scalate
    sf::VideoMode videoMode({ width * scale, height * scale });
    m_window = std::make_unique<sf::RenderWindow>(videoMode, title);

    if (!m_window) {
        std::cerr << "Errore: impossibile creare la finestra" << std::endl;
        return false;
    }

    // 3. Crea la texture per il framebuffer
    m_frameTexture = std::make_unique<sf::Texture>(sf::Vector2u(width, height));
    if (!m_frameTexture) {
        std::cerr << "Errore: impossibile creare la texture" << std::endl;
        return false;
    }

    // 4. Crea lo sprite per visualizzare la texture
    m_frameSprite = std::make_unique<sf::Sprite>(*m_frameTexture);
    m_frameSprite->setScale(sf::Vector2f(scale, scale));  // Applica lo scaling

    std::cout << "SFMLBackend inizializzato: " << width << "x" << height
        << " (scale " << scale << "x)" << std::endl;
    return true;
}

void SFMLBackend::DisplayFrameBuffer(const uint32_t *frameBuffer,
    unsigned int width, unsigned int height,
    int offsetX, int offsetY)
{
    // 1. Valida i parametri
    if (!frameBuffer || !m_frameTexture) {
        std::cerr << "❌ Errore: framebuffer o texture nullo" << std::endl;
        return;
    }

    // 2. Converti il framebuffer RGBA in texture SFML
    // SFML carica i dati così: m_frameTexture->update((uint8_t*)frameBuffer);
    //m_frameTexture->update((uint8_t *)frameBuffer);
    // Converti da RGBA 32-bit a SFML Uint8 RGBA
    // TEST: Riempi la texture di rosso
    std::vector<uint8_t> pixels(width * height * 4);
    for (unsigned int i = 0; i < width * height; i++) {
        uint32_t rgba = frameBuffer[i];

        // Estrai i byte così come sono
        uint8_t byte0 = (rgba) & 0xFF;
        uint8_t byte1 = (rgba >> 8) & 0xFF;
        uint8_t byte2 = (rgba >> 16) & 0xFF;
        uint8_t byte3 = (rgba >> 24) & 0xFF;

        // Metti nel buffer RGBA
        pixels[i * 4 + 0] = byte0;
        pixels[i * 4 + 1] = byte1;
        pixels[i * 4 + 2] = byte2;
        pixels[i * 4 + 3] = byte3;
    }
    m_frameTexture->update(pixels.data());


    // 3. Posiziona lo sprite con offset
    m_frameSprite->setPosition(sf::Vector2f(offsetX, offsetY));

   
}

void SFMLBackend::Present()
{
    // 1. Pulisci lo schermo
    m_window->clear(sf::Color::Black);

    // 2. Disegna lo sprite (il framebuffer)
    m_window->draw(*m_frameSprite);

    // 3. Mostra il risultato
    m_window->display();
}

bool SFMLBackend::IsKeyPressed(KeyCode key)
{
    // Mappa KeyCode (nostro enum) a sf::Keyboard::Key (SFML)
    switch (key) {
    case KeyCode::UP:
        return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up);
    case KeyCode::DOWN:
        return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down);
    case KeyCode::LEFT:
        return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left);
    case KeyCode::RIGHT:
        return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right);
    case KeyCode::SPACE:
        return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);
    case KeyCode::ENTER:
        return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter);
    case KeyCode::P:
        return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::P);
    case KeyCode::ESC:
        return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape);
    default:
        return false;
    }
}

void SFMLBackend::Shutdown()
{
    if (m_window && m_window->isOpen()) {
        m_window->close();
    }
    m_frameSprite.reset();
    m_frameTexture.reset();
    m_window.reset();
    std::cout << "✅ SFMLBackend spento" << std::endl;
}

std::pair<int, int> SFMLBackend::GetWindowSize() const
{
    return std::pair<int, int>(m_width, m_height);
}

std::string SFMLBackend::GetBackendName() const
{
    return "SFML";
}