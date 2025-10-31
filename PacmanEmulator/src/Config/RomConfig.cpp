#include "Config/RomConfig.h"

const std::vector<ROMFile> cpuRoms = {
    {"pm1_prg1.6e", 0x0000, 0x0800},
    {"pm1_prg2.6k", 0x0800, 0x0800},
    {"pm1_prg3.6f", 0x1000, 0x0800},
    {"pm1_prg4.6m", 0x1800, 0x0800},
    {"pm1_prg5.6h", 0x2000, 0x0800},
    {"pm1_prg6.6n", 0x2800, 0x0800},
    {"pm1_prg7.6j", 0x3000, 0x0800},
    {"pm1_prg8.6p", 0x3800, 0x0800}
};

const std::vector<ROMFile> graphicRoms = {
    {"pm1_chg1.5e", 0x0000, 0x0800},
    {"pm1_chg2.5h", 0x0800, 0x0800},
    {"pm1_chg3.5f", 0x1000, 0x0800},
    {"pm1_chg4.5j", 0x1800, 0x0800}
};

const ROMFile graphicsPaletteFile = { "pm1-2.3m", 0x0000, 0x0100 };