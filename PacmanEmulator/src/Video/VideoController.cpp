#include "Video/VideoController.h"
#include <iostream>
#include <fstream>

VideoController::VideoController(MemoryBus &memory) : m_memory(memory), m_tileDecoder(memory)
{
	m_frameBuffer.fill(0);
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