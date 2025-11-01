#include "Video/TileDecoder.h"

TileDecoder::TileDecoder(const MemoryBus &memory): m_memory(memory)
{
}

std::array<uint32_t, 64> TileDecoder::DecodeTile(uint8_t tile_index, uint8_t palette_offset)
{
	std::array<uint32_t, 64>output;
	const uint8_t *tileData = m_memory.GetGraphicsTiles();

	size_t tileOffset = tile_index << 4; // Moltiplicazione per 16

	for (int row = 0; row < 8; row++) {
		// Leggi 2 byte per questa riga
		uint8_t plane1 = tileData[tileOffset + row * 2 + 0];
		uint8_t plane2 = tileData[tileOffset + row * 2 + 1];

		//Decodifica riga
		auto rowPixels = DecodeRow(plane1, plane2, palette_offset);

		//Salva in output array (8 pixel per riga x 8 righe = 64 total
		for (int col = 0; col < 8; col++) {
			output[row * 8 + col] = rowPixels[col];
		}
	}
	return output;
}

uint32_t TileDecoder::ConvertPaletteByteToRGBA(uint8_t palette_byte)
{
	uint8_t red_bit = (palette_byte & 0xE0) >> 5;
	uint8_t green_bit = (palette_byte & 0x1C) >> 2;
	uint8_t blue_bit = palette_byte & 0x03;

	uint8_t red_8bit = red_bit * 255 / 7;
	uint8_t green_8bit = green_bit * 255 / 7;
	uint8_t blue_8bit = blue_bit * 255 / 3;

	uint32_t rgba = (red_8bit << 24) | (green_8bit << 16) | (blue_8bit << 8) | 0xFF;

	return rgba;
}

std::array<uint32_t, 8> TileDecoder::DecodeRow(uint8_t plane0, uint8_t plane1, uint8_t palette_offset)
{
	std::array<uint32_t, 8> decoded_row;
	const uint8_t *paletteData = m_memory.GetGraphicsPalette();

	for (int col = 0; col < 8; col++) {
		uint8_t bit0_col = (plane0 >> col) & 0x01;
		uint8_t bit1_col = (plane1 >> col) & 0x01;
		uint8_t pixel_value = (bit1_col << 1) | bit0_col;
		uint8_t palette_byte = paletteData[palette_offset * 16 + pixel_value];
		decoded_row[col] = ConvertPaletteByteToRGBA(palette_byte);
	}

	return decoded_row;
}
