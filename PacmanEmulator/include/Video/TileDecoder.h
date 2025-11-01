#pragma once

#include <array>
#include "Memory/MemoryBus.h"

class TileDecoder {
public:
	TileDecoder(const MemoryBus &memory);

	std::array<uint32_t, 64> DecodeTile(uint8_t tile_index, uint8_t palette_offset);
private:
	const MemoryBus &m_memory;
	uint32_t ConvertPaletteByteToRGBA(uint8_t palette_byte);
	std::array<uint32_t, 8> DecodeRow(uint8_t plane0, uint8_t plane1, uint8_t palette_offset);
};