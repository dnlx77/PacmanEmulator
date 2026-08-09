#include "Video/TileDecoder.h"

TileDecoder::TileDecoder(const MemoryBus &memory) : m_memory(memory)
{
}

std::array<uint32_t, 64> TileDecoder::DecodeTile(uint16_t tile_index, uint8_t palette_offset)
{
    std::array<uint32_t, 64> output = {};
    const uint8_t *tileData = m_memory.GetGraphicsTiles();
    const uint8_t *paletteData = m_memory.GetGraphicsPalette();
    const uint8_t *paletteLookup = m_memory.GetGraphicsPaletteLookup();

    size_t tileOffset = tile_index * 16;

    for (int y = 0; y < 8; y++) {
        // LOGICA LABIRINTO (Funzionante):
        // Byte basso (0-7) = Metà DESTRA (Pixel 4-7)
        // Byte alto (8-15) = Metà SINISTRA (Pixel 0-3)
        uint8_t byteRight = tileData[tileOffset + y];
        uint8_t byteLeft = tileData[tileOffset + y + 8];

        for (int x = 0; x < 8; x++) {
            uint8_t bit0, bit1;

            if (x < 4) {
                // Metà SINISTRA (byteLeft)
                int bitIndex = 3 - x;
                bit0 = (byteLeft >> bitIndex) & 0x01;
                bit1 = (byteLeft >> (bitIndex + 4)) & 0x01;
            }
            else {
                // Metà DESTRA (byteRight)
                int localX = x - 4;
                int bitIndex = 3 - localX;
                bit0 = (byteRight >> bitIndex) & 0x01;
                bit1 = (byteRight >> (bitIndex + 4)) & 0x01;
            }

            uint8_t pixel_value = (bit1 << 1) | bit0;
            uint8_t lookup_addr = (palette_offset << 2) | pixel_value;
            uint8_t color_index = paletteLookup[lookup_addr] & 0x0F;
            uint32_t final_color = ConvertPaletteByteToRGBA(paletteData[color_index]);

            // Rotazione 90°
            int targetX = (7 - y);
            int targetY = x;
            output[(targetY * 8) + targetX] = final_color;
        }
    }
    return output;
}

std::array<uint32_t, 256> TileDecoder::DecodeSprite(uint16_t sprite_index, uint8_t palette_offset)
{
    // Stessa formula (xoffset/yoffset/planeoffset) della "spritelayout" del
    // driver Pac-Man di MAME (src/mame/pacman/pacman.cpp): ogni sprite e' un
    // blocco di 64 byte, decodificato in bit-plane con la stessa convenzione
    // MSB-first del labirinto (bit 0 della tabella = bit piu' significativo).
    static const int xoffset[16] = {
        8 * 8, 8 * 8 + 1, 8 * 8 + 2, 8 * 8 + 3, 16 * 8 + 0, 16 * 8 + 1, 16 * 8 + 2, 16 * 8 + 3,
        24 * 8 + 0, 24 * 8 + 1, 24 * 8 + 2, 24 * 8 + 3, 0, 1, 2, 3
    };
    static const int yoffset[16] = {
        0 * 8, 1 * 8, 2 * 8, 3 * 8, 4 * 8, 5 * 8, 6 * 8, 7 * 8,
        32 * 8, 33 * 8, 34 * 8, 35 * 8, 36 * 8, 37 * 8, 38 * 8, 39 * 8
    };
    static const int planeoffset[2] = { 0, 4 };

    std::array<uint32_t, 256> output = {};
    // La ROM sprite (pacman.5f) e' mappata subito dopo quella dei tile
    // (pacman.5e, 0x1000 byte) nell'array combinato di GetGraphicsTiles().
    const uint8_t *spriteData = m_memory.GetGraphicsTiles() + 0x1000 + sprite_index * 64;
    const uint8_t *paletteData = m_memory.GetGraphicsPalette();
    const uint8_t *paletteLookup = m_memory.GetGraphicsPaletteLookup();

    auto getBit = [&](int bitOffset) -> uint8_t {
        int byteIdx = bitOffset / 8;
        int bit = 7 - (bitOffset % 8); // MSB-first
        return (spriteData[byteIdx] >> bit) & 0x01;
    };

    // La formula xoffset/yoffset da sola produce l'immagine ruotata di 90°
    // in senso antiorario rispetto all'orientamento reale (verificato
    // confrontando il rendering in gioco). Applichiamo qui la correzione:
    // per il pixel di output (ox,oy), il campione va preso da (x=oy, y=15-ox).
    for (int oy = 0; oy < 16; oy++) {
        for (int ox = 0; ox < 16; ox++) {
            int x = oy;
            int y = 15 - ox;

            uint8_t bit0 = getBit(planeoffset[0] + xoffset[x] + yoffset[y]);
            uint8_t bit1 = getBit(planeoffset[1] + xoffset[x] + yoffset[y]);
            uint8_t pixel_value = (bit1 << 1) | bit0;

            uint8_t lookup_addr = (palette_offset << 2) | pixel_value;
            uint8_t color_index = paletteLookup[lookup_addr] & 0x0F;
            output[oy * 16 + ox] = ConvertPaletteByteToRGBA(paletteData[color_index]);
        }
    }
    return output;
}

// TileDecoder.cpp

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