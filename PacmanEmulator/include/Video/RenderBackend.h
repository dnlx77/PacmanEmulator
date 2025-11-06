#pragma once
#include <string>

enum class KeyCode {
	UP, DOWN, LEFT, RIGHT,
	SPACE, ENTER,
	P, // Pause
	ESC // Quit
};

class RenderBackend {
public:
	virtual ~RenderBackend() = default;

	/// Inizializza il backend di rendering
	/// @param width Larghezza della finestra in pixel
	/// @param height Altezza della finestra in pixel
	/// @param scale Moltiplicatore di scala (1x, 2x, 3x, ecc.)
	/// @param title Titolo della finestra
	/// @return true se inizializzazione riuscita, false altrimenti
	virtual bool Initialize(unsigned int width, unsigned int height, unsigned int scale,
		const std::string &title) = 0;

	/// Visualizza il framebuffer a schermo
	/// @param frameBuffer Pointer ai dati RGBA 32-bit
	/// @param width Larghezza del framebuffer in pixel
	/// @param height Altezza del framebuffer in pixel
	/// @param offsetX Offset orizzontale (default 0)
	/// @param offsetY Offset verticale (default 0)
	virtual void DisplayFrameBuffer(const uint32_t *frameBuffer,
		unsigned int width, unsigned int height,
		int offsetX = 0, int offsetY = 0) = 0;

	/// Presenta il frame a schermo (swapchain)
	virtual void Present() = 0;

	/// Verifica se un tasto è attualmente premuto
	/// @param key Il tasto da controllare
	/// @return true se il tasto è premuto, false altrimenti
	virtual bool IsKeyPressed(KeyCode key) = 0;

	/// Chiude il backend e libera le risorse
	virtual void Shutdown() = 0;

	/// Ritorna le dimensioni della finestra
	/// @return Pair (width, height) in pixel
	virtual std::pair<int, int> GetWindowSize() const = 0;

	/// Ritorna il nome del backend implementato
	/// @return Stringa con il nome (es. "SFML", "SDL2")
	virtual std::string GetBackendName() const = 0;
};