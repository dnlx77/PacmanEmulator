#include "Config/RomConfig.h"

const std::vector<ROMFile> cpuRoms = {
    {"pacman.6e", 0x0000, 0x1000},
    {"pacman.6f", 0x1000, 0x1000},
    {"pacman.6h", 0x2000, 0x1000},
    {"pacman.6j", 0x3000, 0x1000},
};

const std::vector<ROMFile> graphicRoms = {
    {"pacman.5e", 0x0000, 0x1000},
    {"pacman.5f", 0x1000, 0x1000},
};

const ROMFile graphicsPaletteFile = { "82s123.7f", 0x0000, 0x0020 };

const ROMFile graphicsPaletteLookupFile = { "82s126.4a", 0x0000, 0x0100 };