#pragma once

#include "Memory/MemoryBus.h"
#include "Video/TileDecoder.h"

static constexpr int SCREEN_WIDTH = 224;
static constexpr int SCREEN_HEIGHT = 288;
static constexpr int TILE_FOR_ROW = 28;
static constexpr int TILE_FOR_COL = 36;
static constexpr int SCREEN_SIZE = SCREEN_WIDTH * SCREEN_HEIGHT;

class VideoController {
public:
	VideoController(MemoryBus &memory);
	void RenderScanline(int scanline_y);
	void RenderFrame();
	bool SaveFramebufferPPM(const std::string &filename) const;
	const uint32_t* GetFrameBuffer() const;
	std::pair<int, int> GetFrameBufferSize() const;

private:
	MemoryBus &m_memory;
	TileDecoder m_tileDecoder;
	std::array<uint32_t, SCREEN_SIZE> m_frameBuffer;
	void RenderTile(int tile_x, int tile_y);
};
