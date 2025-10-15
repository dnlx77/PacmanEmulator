#pragma once
#include<cstdint>
#include<string>
#include<vector>
#include<array>

class MemoryBus {
private:
	std::array<uint8_t, 0X4000> m_rom;		// Rom
	std::array<uint8_t, 0X400> m_VRam;		// Video Ram
	std::array<uint8_t, 0X400> m_CRam;		// Color Ram
	std::array<uint8_t, 0X400> m_ram;		// Ram
	std::array<uint8_t, 0X100> m_SRam;		// Sprite ram

	bool m_romLoaded;
public:
	MemoryBus();
	~MemoryBus();
	bool IsROMLoaded() const { return m_romLoaded; }
	uint8_t Read(uint16_t address);
	void Write(uint16_t address, uint8_t value);
	void Initialize();
	bool LoadRom(const std::string &filename);
};