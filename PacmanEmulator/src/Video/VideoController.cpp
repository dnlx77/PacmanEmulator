#include "Video/VideoController.h"
#include <iostream>
#include <fstream>

VideoController::VideoController(MemoryBus &memory) : m_memory(memory), m_tileDecoder(memory)
{
	m_frameBuffer.fill(0);
}

void VideoController::RenderScanline(int scanline_y)
{
	// 1. Calcola quale riga di tile siamo
	int tile_row = scanline_y / 8;

	// 2. Calcola quale riga DENTRO il tile (0-7)
	int pixel_row_in_tile = scanline_y % 8;

	// 3. Per ogni tile_x (0-27):
	for (int tile_x = 0; tile_x < TILE_FOR_ROW; tile_x++) {
		// 3a. Leggi tile_index dalla VRAM
		int tile_offset = tile_row * 28 + tile_x;
		uint8_t tile_index = m_memory.Read(0x4000+tile_offset);

		// 3b. Leggi colore dalla color RAM
		uint8_t color_index = m_memory.Read(0x4400 + tile_offset);

		// 3c. Decodi il tile
		auto tile_pixels = m_tileDecoder.DecodeTile(tile_index, color_index);

		// 3d. Estrai la riga corretta del tile (pixel_row_in_tile)
		// tile_pixels è un array<uint32_t, 64> = 8×8 pixel
		// Devi estrarre solo la riga pixel_row_in_tile

		// 3e. Scrivi i pixel nel framebuffer
		for (int px = 0; px < 8; px++) {
			int screen_x = tile_x * 8 + px;
			int screen_y = scanline_y;
			int fb_offset = screen_y * SCREEN_WIDTH + screen_x;

			// ← Quale pixel del tile usi qui?
			m_frameBuffer[fb_offset] = tile_pixels[pixel_row_in_tile * 8 + px];
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
	int tile_offset = tile_y * 28 + tile_x;
	uint8_t tile_index = m_memory.Read(0x4000 + tile_offset);
	uint8_t color_index = m_memory.Read(0x4400 + tile_offset);

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