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
	std::array<uint8_t, 0X0800> m_ram;		// Ram
	std::array<uint8_t, 0X100> m_SRam;		// Sprite ram
	std::array<uint8_t, 0x2000> m_graphicsTiles;
	std::array<uint8_t, 0x100> m_graphicsPalette;
	std::array<uint8_t, 0x100> m_paletteLookup;

	// I/O Ports
	uint8_t m_in0 = 0xFF;        // Joystick P1, coin, etc.
	uint8_t m_in1 = 0xFF;        // Joystick P2, start buttons
	uint8_t m_dipSwitches = 0xC9; // DIP switches config

	bool m_irqEnabled = false;

	// Sprite data (la VERA sprite RAM)
	std::array<uint8_t, 16> m_spriteCoords;   // 0x5060-0x506F
	std::array<uint8_t, 16> m_spriteAttribs;  // 0x5060-0x506F (different view)

public:
	enum class ROMType {
		CPU,
		GRAPHICS_TILES,
		GRAPHICS_PALETTE,
		PALETTE_LOOKUP
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
	const uint8_t *GetGraphicsPaletteLookup() const;

	bool IsIrqEnabled() const { return m_irqEnabled; }
};