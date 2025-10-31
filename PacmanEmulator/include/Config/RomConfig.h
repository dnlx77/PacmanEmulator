#pragma once
#include<string>
#include<vector>

struct ROMFile {
    std::string filename;
    size_t offset;
    size_t expectedSize;  // Per validation
};

extern const std::vector<ROMFile> cpuRoms;
extern const std::vector<ROMFile> graphicRoms;
extern const ROMFile graphicsPaletteFile;

