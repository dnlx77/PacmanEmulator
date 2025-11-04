#include "Memory/MemoryBus.h"
#include <fstream>
#include <iostream>
#include <iomanip>

MemoryBus::MemoryBus()
{
}

MemoryBus::~MemoryBus()
{
}

uint8_t MemoryBus::Read(uint16_t address)
{
	// ROM: read-only
	if (address <= 0x3FFF) return m_rom[address];

	// Video RAM: 0x4000 - 0x43FF
	if (address <= 0x43FF) return m_VRam[address - 0x4000];

	// Color RAM: 0x4400 - 0x47FF
	if (address <= 0x47FF) return m_CRam[address - 0x4400];

	// RAM: 0x4800 - 0x4FFF (con mirroring)
	if (address <= 0x4FFF) return m_ram[(address - 0x4800) % 0x0400];

	// Sprite RAM: 0x5000 - 0x50FF
	if (address <= 0x50FF) return m_SRam[address - 0x5000];

	// Unmapped - STAMPA!
	std::cerr << "WARNING: Read from unmapped address 0x"
		<< std::hex << std::setfill('0') << std::setw(4)
		<< address << std::dec << "\n";
	return 0xFF;
	
}

void MemoryBus::Write(uint16_t address, uint8_t value)
{
	// ROM: read-only
	if (address <= 0x3FFF) {
		std::cerr << "WARNING: Write to ROM at 0x"
			<< std::hex << std::setfill('0') << std::setw(4)
			<< address << " = 0x" << std::setw(2) << (int)value
			<< std::dec << "\n";
		return;
	}

	// Video RAM: 0x4000 - 0x43FF
	if (address <= 0x43FF) { m_VRam[address - 0x4000] = value; return; }

	// Color RAM: 0x4400 - 0x47FF
	if (address <= 0x47FF) { m_CRam[address - 0x4400] = value; return; }
	

	// RAM: 0x4800 - 0x4FFF (con mirroring)
	if (address <= 0x4FFF) { m_ram[(address - 0x4800) % 0x0400] = value; return; }

	// Sprite RAM: 0x5000 - 0x50FF
	if (address <= 0x50FF) { m_SRam[address - 0x5000] = value; return; }

	// Unmapped - STAMPA!
	std::cerr << "WARNING: Write to unmapped address 0x"
		<< std::hex << std::setfill('0') << std::setw(4)
		<< address << " = 0x" << std::setw(2) << (int)value
		<< std::dec << "\n";
}

void MemoryBus::Initialize() {
	m_rom.fill(0);
	m_VRam.fill(0);
	m_CRam.fill(0);
	m_ram.fill(0);
	m_SRam.fill(0);
	m_graphicsTiles.fill(0);
	m_graphicsPalette.fill(0);
}

size_t MemoryBus::LoadRom(const std::string &filename, ROMType type, size_t offset)
{
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Errore: impossibile aprire " << filename << std::endl;
        return 0;
    }

    // Ottieni dimensione
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::cout << "Dimensione file: " << fileSize << " bytes" << std::endl;
	
	if (type == ROMType::CPU) {
		// Leggi i dati
		file.read(reinterpret_cast<char *>(m_rom.data() + offset), fileSize);
	}
	else if (type == ROMType::GRAPHICS_TILES) {
		file.read(reinterpret_cast<char *>(m_graphicsTiles.data() + offset), fileSize);
	}
	else if (type == ROMType::GRAPHICS_PALETTE) {
		file.read(reinterpret_cast<char *>(m_graphicsPalette.data() + offset), fileSize);
	}
	else if (type==ROMType::PALETTE_LOOKUP) {
		file.read(reinterpret_cast<char *>(m_paletteLookup.data() + offset), fileSize);
	}
    
	size_t bytesRead = file.gcount();
    file.close();

    std::cout << "ROM caricata con successo: " << filename << std::endl;
    
	return bytesRead;
}

const uint8_t *MemoryBus::GetGraphicsTiles() const
{
	return m_graphicsTiles.data();
}

const uint8_t *MemoryBus::GetGraphicsPalette() const
{
	return m_graphicsPalette.data();
}

const uint8_t *MemoryBus::GetGraphicsPaletteLookup() const
{
	return m_paletteLookup.data();
}
