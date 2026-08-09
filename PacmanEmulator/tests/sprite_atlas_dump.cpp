// Tool diagnostico: assembla i 64 sprite 16x16 (4 quarti 8x8 ciascuno) dalla
// ROM sprite (pacman.5f) usando diverse combinazioni di bit-order e mappatura
// quadranti, per confrontare visivamente quale produce forme riconoscibili
// (Pac-Man, fantasmi) senza dover rilanciare l'intero emulatore.
//
// Uso: sprite_atlas_dump <assets_dir> <bitorder 0=MSB|1=LSB> <quadmap 0|1> <output.pgm>

#include <cstdint>
#include <fstream>
#include <vector>
#include <string>
#include <iostream>

static std::vector<uint8_t> loadFile(const std::string &path, size_t expectedSize)
{
    std::ifstream f(path, std::ios::binary);
    std::vector<uint8_t> data(expectedSize, 0);
    if (!f.is_open()) {
        std::cerr << "Impossibile aprire " << path << "\n";
        return data;
    }
    f.read(reinterpret_cast<char *>(data.data()), expectedSize);
    return data;
}

// Decodifica un tile 8x8 (16 byte) in un buffer 8x8 di valori 0-3.
static void decodeTile(const uint8_t *tileData, int bitorder, uint8_t out[64])
{
    for (int y = 0; y < 8; y++) {
        uint8_t byteRight = tileData[y];
        uint8_t byteLeft = tileData[y + 8];

        for (int x = 0; x < 8; x++) {
            uint8_t b = (x < 4) ? byteLeft : byteRight;
            int localX = (x < 4) ? x : (x - 4);

            int bitIndex = (bitorder == 0) ? (3 - localX) : localX; // 0=MSB,1=LSB
            uint8_t bit0 = (b >> bitIndex) & 1;
            uint8_t bit1 = (b >> (bitIndex + 4)) & 1;
            uint8_t pixel = (bit1 << 1) | bit0;

            int targetX = 7 - y;
            int targetY = x;
            out[targetY * 8 + targetX] = pixel;
        }
    }
}

int main(int argc, char **argv)
{
    if (argc < 5) {
        std::cerr << "Uso: " << argv[0] << " <assets_dir> <bitorder 0|1> <quadmap 0|1> <output.pgm>\n";
        return 1;
    }

    std::string assetsDir = argv[1];
    int bitorder = std::stoi(argv[2]);
    int quadmap = std::stoi(argv[3]);
    std::string outPath = argv[4];

    auto sf = loadFile(assetsDir + "/pacman.5f", 0x1000);

    const int SPRITES = 64;
    const int COLS = 8;
    const int ROWS = 8;
    const int SPRITE_PX = 16;
    const int SCALE = 4;

    int imgW = COLS * SPRITE_PX * SCALE;
    int imgH = ROWS * SPRITE_PX * SCALE;
    std::vector<uint8_t> img(imgW * imgH, 0);

    for (int s = 0; s < SPRITES; s++) {
        int base = s * 4; // 4 tile consecutivi per sprite, come (tile_info&0xFC)

        uint8_t q[4][64];
        decodeTile(sf.data() + (base + 0) * 16, bitorder, q[0]);
        decodeTile(sf.data() + (base + 1) * 16, bitorder, q[1]);
        decodeTile(sf.data() + (base + 2) * 16, bitorder, q[2]);
        decodeTile(sf.data() + (base + 3) * 16, bitorder, q[3]);

        // quadmap 0: TL=2,TR=0,BL=3,BR=1 (mappatura originale del gioco)
        // quadmap 1: TL=0,TR=2,BL=1,BR=3 (ipotesi alternativa)
        uint8_t *TL, *TR, *BL, *BR;
        if (quadmap == 0) { TL = q[2]; TR = q[0]; BL = q[3]; BR = q[1]; }
        else              { TL = q[0]; TR = q[2]; BL = q[1]; BR = q[3]; }

        int sx = (s % COLS) * SPRITE_PX;
        int sy = (s / COLS) * SPRITE_PX;

        auto blit = [&](uint8_t *quad, int ox, int oy) {
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    uint8_t v = quad[y * 8 + x];
                    uint8_t gray = static_cast<uint8_t>(v * 85);
                    int px = sx + ox + x;
                    int py = sy + oy + y;
                    for (int dy = 0; dy < SCALE; dy++) {
                        for (int dx = 0; dx < SCALE; dx++) {
                            img[(py * SCALE + dy) * imgW + (px * SCALE + dx)] = gray;
                        }
                    }
                }
            }
        };
        blit(TL, 0, 0);
        blit(TR, 8, 0);
        blit(BL, 0, 8);
        blit(BR, 8, 8);
    }

    std::ofstream out(outPath, std::ios::binary);
    out << "P5\n" << imgW << " " << imgH << "\n255\n";
    out.write(reinterpret_cast<char *>(img.data()), img.size());

    std::cout << "Scritto " << outPath << " (bitorder=" << bitorder << " quadmap=" << quadmap << ")\n";
    return 0;
}
