#include "Video/VideoController.h"
#include <iostream>
#include <fstream>

VideoController::VideoController(MemoryBus &memory) : m_memory(memory), m_tileDecoder(memory)
{
	m_frameBuffer.fill(0);
}

void VideoController::RenderScanline(int scanline_y)
{
	// Pac-Man ha risoluzione 224x288
	if (scanline_y >= SCREEN_HEIGHT) return;

	int tile_y = scanline_y / 8;        // Riga del tile (0-35)
	int pixel_row_in_tile = scanline_y % 8;

	// Per ogni colonna dello schermo (0-27)
	for (int tile_x = 0; tile_x < 28; tile_x++) {

		// --- FIX: Calcolo Indirizzo VRAM Corretto ---
		uint16_t vram_offset = GetVramOffset(tile_x, tile_y);
		// --------------------------------------------

		// Leggi Tile Index (0x4000) e Attributi Colore (0x4400) usando lo stesso offset
		uint8_t tile_index = m_memory.Read(0x4000 + vram_offset);
		uint8_t color_attr = m_memory.Read(0x4400 + vram_offset); // Era color_index

		// Decodifica: Attenzione! Passa 'color_attr' (che è l'indice palette), 
		// NON tile_index o altro.
		auto tile_pixels = m_tileDecoder.DecodeTile(tile_index, color_attr);

		// Disegna gli 8 pixel orizzontali
		for (int px = 0; px < 8; px++) {
			int screen_x = tile_x * 8 + px;
			int screen_y = scanline_y;

			// Check bounds (sicurezza)
			if (screen_x < SCREEN_WIDTH && screen_y < SCREEN_HEIGHT) {
				int fb_offset = screen_y * SCREEN_WIDTH + screen_x;
				m_frameBuffer[fb_offset] = tile_pixels[pixel_row_in_tile * 8 + px];
			}
		}
	}
}

void VideoController::RenderFrame()
{
	for (int y = 0; y < 36; y++) {
		for (int x = 0; x < 28; x++) {
			RenderTile(x, y);
		}
	}
}

const uint32_t *VideoController::GetFrameBuffer() const
{
	return m_frameBuffer.data();
}

std::pair<int, int> VideoController::GetFrameBufferSize() const
{
	return { SCREEN_WIDTH, SCREEN_HEIGHT };
}

void VideoController::RenderTile(int tile_x, int tile_y)
{
	// Usa la stessa formula helper
	uint16_t vram_offset = GetVramOffset(tile_x, tile_y);

	uint8_t tile_index = m_memory.Read(0x4000 + vram_offset);
	uint8_t color_index = m_memory.Read(0x4400 + vram_offset);

	auto tile_pixels = m_tileDecoder.DecodeTile(tile_index, color_index);

	for (int py = 0; py < 8; py++) {
		for (int px = 0; px < 8; px++) {
			int screen_x = tile_x * 8 + px;
			int screen_y = tile_y * 8 + py;
			int fb_offset = screen_y * SCREEN_WIDTH + screen_x;

			m_frameBuffer[fb_offset] = tile_pixels[py * 8 + px];
		}
	}
}

uint16_t VideoController::GetVramOffset(int x, int y)
{
	// x: 0..27 (Screen coordinates)
	// y: 0..35 (Screen coordinates)

	// 1. Area BOTTOM (Righe 34, 35 - Status/Vite)
	// Indirizzi REALI: 0x0000 - 0x003F (Le prime due colonne della VRAM)
	if (y >= 34) {
		// Offset 0x000.
		// Usiamo '29 - x' per mantenere la direzione corretta del testo.
		// Nota: Le righe 34/35 mappano alle colonne logiche VRAM 0 e 1.
		return 0x000 + (29 - x) + ((y - 34) * 32);
	}

	// 2. Area TOP (Righe 0, 1 - Punteggio)
	// Indirizzi REALI: 0x03C0 - 0x03FF (Le ultime due colonne della VRAM)
	else if (y < 2) {
		// Offset 0x3C0.
		return 0x3C0 + (29 - x) + (y * 32);
	}

	// 3. Area MIDDLE (Righe 2-33 - Labirinto)
	// Indirizzi REALI: 0x0040 - 0x03BF
	else {
		// Questa parte era già corretta (0x40+), ma la ripetiamo per chiarezza.
		// Colonna 29 (x=0) -> 29*32 = 928 (0x3A0)
		// Colonna 2 (x=27) -> 2*32  = 64  (0x040)
		return (29 - x) * 32 + (y - 2);
	}
}

bool VideoController::SaveFramebufferPPM(const std::string &filename) const
{
	std::ofstream file(filename, std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "Cannot open " << filename << "\n";
		return false;
	}

	file << "P6\n";
	file << SCREEN_WIDTH << " " << SCREEN_HEIGHT << "\n";
	file << "255\n";

	for (int i = 0; i < SCREEN_SIZE; i++) {
		uint32_t rgba = m_frameBuffer[i];
		uint8_t r = (rgba >> 24) & 0xFF;
		uint8_t g = (rgba >> 16) & 0xFF;
		uint8_t b = (rgba >> 8) & 0xFF;

		file.put(r);
		file.put(g);
		file.put(b);
	}

	file.close();
	std::cout << "Framebuffer saved to " << filename << "\n";
	return true;
}