#pragma once
#include "RenderBackend.h"
#include <SFML/Graphics.hpp>
#include <memory>

class SFMLBackend :public RenderBackend {
public:
	SFMLBackend();
	~SFMLBackend() override;

	// Delete copy
	SFMLBackend(const SFMLBackend &) = delete;
	SFMLBackend &operator= (const SFMLBackend &) = delete;

	// Implementazione dei metodi virtuali
	bool Initialize(unsigned int width, unsigned int height, unsigned int scale,
		const std::string &title) override;
	void DisplayFrameBuffer(const uint32_t *frameBuffer,
		unsigned int width, unsigned int height,
		int offsetX = 0, int offsetY = 0) override;
	void Present() override;
	bool IsKeyPressed(KeyCode key) override;
	void Shutdown() override;
	std::pair<int, int> GetWindowSize() const override;
	std::string GetBackendName() const override;
private:

	std::unique_ptr<sf::RenderWindow> m_window;
	std::unique_ptr<sf::Texture> m_frameTexture;
	std::unique_ptr<sf::Sprite> m_frameSprite;
	std::string m_win_title;
	unsigned int m_width, m_height;
	unsigned int m_scale;
};