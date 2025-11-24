#include "Video/TileDecoder.h"

TileDecoder::TileDecoder(const MemoryBus &memory) : m_memory(memory)
{
}

std::array<uint32_t, 64> TileDecoder::DecodeTile(uint8_t tile_index, uint8_t palette_offset)
{
    std::array<uint32_t, 64> output = {};
    const uint8_t *tileData = m_memory.GetGraphicsTiles();
    const uint8_t *paletteData = m_memory.GetGraphicsPalette();
    const uint8_t *paletteLookup = m_memory.GetGraphicsPaletteLookup();

    size_t tileOffset = tile_index << 4;

    for (int y = 0; y < 8; y++) {
        // Leggi i due byte (Destra e Sinistra)
        uint8_t byteRight = tileData[tileOffset + y];
        uint8_t byteLeft = tileData[tileOffset + y + 8];

        for (int x = 0; x < 8; x++) {
            uint8_t bit0, bit1;

            // --- FIX SPECCHIAMENTO ---
            // Dobbiamo leggere i bit in ordine inverso (MSB first)
            // Invece di 'bitIndex = x', usiamo 'bitIndex = 3 - x'

            if (x < 4) {
                // Metà SINISTRA (byteLeft)
                int bitIndex = 3 - x; // <--- FIX QUI (Era 'x')

                bit0 = (byteLeft >> bitIndex) & 0x01;
                bit1 = (byteLeft >> (bitIndex + 4)) & 0x01;
            }
            else {
                // Metà DESTRA (byteRight)
                int localX = x - 4;
                int bitIndex = 3 - localX; // <--- FIX QUI (Era 'localX')

                bit0 = (byteRight >> bitIndex) & 0x01;
                bit1 = (byteRight >> (bitIndex + 4)) & 0x01;
            }

            uint8_t pixel_value = (bit1 << 1) | bit0;

            // Lookup Colore
            uint8_t lookup_addr = (palette_offset << 2) | pixel_value;
            uint8_t color_index = paletteLookup[lookup_addr] & 0x0F;
            uint32_t final_color = ConvertPaletteByteToRGBA(paletteData[color_index]);

            // Rotazione 90 gradi oraria (già corretta)
            int targetX = (7 - y);
            int targetY = x;

            output[(targetY * 8) + targetX] = final_color;
        }
    }
    return output;
}

uint32_t TileDecoder::ConvertPaletteByteToRGBA(uint8_t palette_byte)
{
    // Hardware Pac-Man (PROM 82S123) output mapping:
    // Bit 0-2: Rosso (peso 1, 2, 4)
    // Bit 3-5: Verde (peso 1, 2, 4)
    // Bit 6-7: Blu   (peso 1, 2)

    uint8_t r = (palette_byte & 0x07);       // Bit 0-2
    uint8_t g = (palette_byte & 0x38) >> 3;  // Bit 3-5
    uint8_t b = (palette_byte & 0xC0) >> 6;  // Bit 6-7

    // Scala i valori da 0-7 (o 0-3 per il blu) a 0-255
    uint8_t red_8bit = (r * 255) / 7;
    uint8_t green_8bit = (g * 255) / 7;
    uint8_t blue_8bit = (b * 255) / 3;

    // Componi RGBA (Alpha sempre 255)
    uint32_t rgba = (red_8bit) | (green_8bit << 8) | (blue_8bit << 16) | (0xFF << 24);
    return rgba;
}

std::array<uint32_t, 8> TileDecoder::DecodeRow(uint8_t plane0, uint8_t plane1, uint8_t palette_offset)
{
    std::array<uint32_t, 8> decoded_row = {};
    const uint8_t *paletteData = m_memory.GetGraphicsPalette();
    const uint8_t *paletteLookup = m_memory.GetGraphicsPaletteLookup();

    for (int col = 0; col < 8; col++) {
        // Usa (7 - col) per leggere dal bit più significativo (MSB) a sinistra
        uint8_t bit0 = (plane0 >> (7 - col)) & 0x01;
        uint8_t bit1 = (plane1 >> (7 - col)) & 0x01;
        uint8_t pixel_value = (bit1 << 1) | bit0;

        // Calcolo indirizzo Lookup e Colore (come visto prima)
        uint8_t lookup_addr = (palette_offset << 2) | pixel_value;
        uint8_t color_index = paletteLookup[lookup_addr] & 0x0F;
        uint8_t palette_byte = paletteData[color_index];

        decoded_row[col] = ConvertPaletteByteToRGBA(palette_byte);
    }
    return decoded_row;
}