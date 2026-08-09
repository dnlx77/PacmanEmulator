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

void VideoController::RenderSprites()
{
	for (int i = 7; i >= 0; i--) {
		uint16_t coord_addr = 0x5060 + (i * 2);
		uint16_t attr_addr = 0x4FF0 + (i * 2);

		// USA PEEK INVECE DI READ!
		// Questo recupera le coordinate vere senza confondere la CPU
		uint8_t x_raw = m_memory.PeekVideoRegister(coord_addr);
		uint8_t y_raw = m_memory.PeekVideoRegister(coord_addr + 1);

		uint8_t tile_info = m_memory.Read(attr_addr);     // Byte 0: Index + Flips
		uint8_t palette_info = m_memory.Read(attr_addr + 1); // Byte 1: Palette

		// Indice sprite hardware 0-63 (4 tile da 16 byte = 64 byte per sprite).
		uint16_t sprite_index = (tile_info & 0xFC) >> 2;

		bool flipX = (tile_info & 0x02) != 0;
		bool flipY = (tile_info & 0x01) != 0;

		uint8_t palette_offset = palette_info & 0x3F;

		int x_screen = 239 - x_raw;
		int y_screen = 272 - y_raw;
		if (x_screen > 224) x_screen -= 256;
		if (y_screen > 288) y_screen -= 256;

		DrawSprite(x_screen, y_screen, sprite_index, palette_offset, flipX, flipY);
	}
}

// VideoController.cpp

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

void VideoController::DrawSprite(int start_x, int start_y, uint16_t sprite_index, uint8_t palette_offset, bool flipX, bool flipY)
{
	auto pixels = m_tileDecoder.DecodeSprite(sprite_index, palette_offset);

	for (int py = 0; py < 16; py++) {
		int srcY = flipY ? (15 - py) : py;

		for (int px = 0; px < 16; px++) {
			int srcX = flipX ? (15 - px) : px;
			uint32_t color = pixels[srcY * 16 + srcX];

			if ((color & 0x00FFFFFF) == 0) continue;

			int screen_x = start_x + px;
			int screen_y = start_y + py;

			if (screen_x >= 0 && screen_x < SCREEN_WIDTH &&
				screen_y >= 0 && screen_y < SCREEN_HEIGHT) {
				m_frameBuffer[screen_y * SCREEN_WIDTH + screen_x] = color;
			}
		}
	}
}

// VideoController.cpp

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
		uint8_t r = rgba & 0xFF;
		uint8_t g = (rgba >> 8) & 0xFF;
		uint8_t b = (rgba >> 16) & 0xFF;

		file.put(r);
		file.put(g);
		file.put(b);
	}

	file.close();
	std::cout << "Framebuffer saved to " << filename << "\n";
	return true;
}