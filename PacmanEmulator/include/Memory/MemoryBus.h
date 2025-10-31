#pragma once
#include<cstdint>
#include<string>
#include<vector>
#include<array>
#include "Config/RomConfig.h"

class MemoryBus {
private:
	std::array<uint8_t, 0X4000> m_rom;		// Rom
	std::array<uint8_t, 0X400> m_VRam;		// Video Ram
	std::array<uint8_t, 0X400> m_CRam;		// Color Ram
	std::array<uint8_t, 0X400> m_ram;		// Ram
	std::array<uint8_t, 0X100> m_SRam;		// Sprite ram
	std::array<uint8_t, 0x2000> m_graphicsTiles;
	std::array<uint8_t, 0x100> m_graphicsPalette;

public:
	enum class ROMType {
		CPU,
		GRAPHICS_TILES,
		GRAPHICS_PALETTE
	};

	MemoryBus();
	~MemoryBus();

	uint8_t Read(uint16_t address);
	void Write(uint16_t address, uint8_t value);
	void Initialize();
	size_t LoadRom(const std::string &filename, ROMType type, size_t offset = 0);
	
	// Getter per VideoController
	const uint8_t *GetGraphicsTiles() const;
	const uint8_t *GetGraphicsPalette() const;
};