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

// MemoryBus.cpp - Read corretto:
uint8_t MemoryBus::Read(uint16_t address)
{
	if (m_flatMode) return m_flatRam[address];

	// ROM: 0x0000-0x3FFF
	if (address <= 0x3FFF) return m_rom[address];

	// Video RAM: 0x4000-0x43FF
	if (address <= 0x43FF) return m_VRam[address - 0x4000];

	// Color RAM: 0x4400-0x47FF
	if (address <= 0x47FF) return m_CRam[address - 0x4400];

	// RAM: 0x4800-0x4FFF
	if (address <= 0x4FFF) return m_ram[address - 0x4800];

	// I/O AREA: 0x5000-0x50FF
	if (address >= 0x5000 && address <= 0x50FF) {
		uint8_t offset = address & 0xFF;

		// --- FIX CRITICO: RIMOSSO BLOCCO SPRITE COORDS ---
		// Le coordinate (5060-506F) sono WRITE-ONLY.
		// In lettura, quest'area è un mirror di IN1 (5040-507F).

		// Gestione Input con Mirroring corretto
		if (offset < 0x40) {
			// 0x5000 - 0x503F: IN0 (Joystick P1, Coins)
			return m_in0;
		}
		if (offset < 0x80) {
			// 0x5040 - 0x507F: IN1 (Start, Service, Cockt P2)
			// Nota: Questo copre anche l'area 5060-506F!
			return m_in1;
		}
		if (offset < 0xC0) {
			// 0x5080 - 0x50BF: Dip Switches
			return m_dipSwitches;
		}

		// 0x50C0 - 0x50FF: Watchdog / Sound (solitamente open bus/FF)
		return 0xFF;
	}

	return 0xFF;
}
// MemoryBus.cpp - Write corretto:
void MemoryBus::Write(uint16_t address, uint8_t value)
{
	if (m_flatMode) { m_flatRam[address] = value; return; }

	// ROM: read-only
	if (address <= 0x3FFF) return;

	// Video RAM: 0x4000-0x43FF
	if (address <= 0x43FF) {
		m_VRam[address - 0x4000] = value;
		return;
	}

	// Color RAM: 0x4400-0x47FF
	if (address <= 0x47FF) {
		m_CRam[address - 0x4400] = value;
		return;
	}

	// RAM: 0x4800-0x4FFF
	if (address <= 0x4FFF) {
		m_ram[address - 0x4800] = value;
		return;
	}

	// I/O AREA: 0x5000-0x50FF
	if (address >= 0x5000 && address <= 0x50FF) {
		uint8_t offset = address & 0xFF;

		// Interrupt Enable: 0x5000 (Write)
		if (offset == 0x00) {
			// Bit 0 controlla l'abilitazione degli interrupt hardware
			m_irqEnabled = (value & 0x01) != 0;
			return;
		}

		// Sound registers: 0x5040-0x505F
		if (offset >= 0x40 && offset < 0x60) {
			// Per ora ignora i suoni
			return;
		}

		// Sprite coordinates: 0x5060-0x506F  
		if (offset >= 0x60 && offset < 0x70) {
			m_spriteCoords[offset - 0x60] = value;
			return;
		}

		// Watchdog: 0x50C0
		if (offset == 0xC0) {
			// Reset watchdog timer (ignora per ora)
			return;
		}

		return;
	}
}

void MemoryBus::Initialize() {
	m_rom.fill(0);
	m_VRam.fill(0);
	m_CRam.fill(0);
	m_ram.fill(0);
	m_SRam.fill(0);
	m_graphicsTiles.fill(0);
	m_graphicsPalette.fill(0);

	m_irqEnabled = false;

	// Setup input per attract mode
	m_in0 = 0xFF;  // Bit pattern: 0011 1111
	// Bit 7: unused
	// Bit 6: coin (1=not pressed)
	// Bit 5-0: altri controlli

	m_in1 = 0xFF;  // Tutti i tasti non premuti

	m_dipSwitches = 0xC9;  // Config standard:
	// 1 coin = 1 credit
	// 3 lives
	// Normal difficulty
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

void MemoryBus::UpdateInputs(uint8_t in0, uint8_t in1)
{
	m_in0 = in0;
	m_in1 = in1;
}

uint8_t MemoryBus::PeekVideoRegister(uint16_t address) const
{
	// Se l'indirizzo è nell'area Sprite Coordinates (0x5060-0x506F)
	if (address >= 0x5060 && address <= 0x506F) {
		return m_spriteCoords[address - 0x5060];
	}

	// Per tutto il resto (es. RAM attributi 4FF0), usiamo l'accesso diretto
	if (address >= 0x4800 && address <= 0x4FFF) {
		return m_ram[address - 0x4800];
	}

	// Per tutto il resto (es. VRAM), usiamo l'accesso diretto
	if (address <= 0x3FFF) return m_rom[address];
	if (address <= 0x43FF) return m_VRam[address - 0x4000];
	if (address <= 0x47FF) return m_CRam[address - 0x4400];
	if (address <= 0x4FFF) return m_ram[address - 0x4800];

	return 0;
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

void MemoryBus::EnableFlatMemoryMode()
{
	m_flatMode = true;
	m_flatRam.fill(0);
}

bool MemoryBus::LoadFlatBinary(const std::string &filename, uint16_t loadAddress)
{
	std::ifstream file(filename, std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "Errore: impossibile aprire " << filename << std::endl;
		return false;
	}

	file.seekg(0, std::ios::end);
	size_t fileSize = static_cast<size_t>(file.tellg());
	file.seekg(0, std::ios::beg);

	if (static_cast<size_t>(loadAddress) + fileSize > m_flatRam.size()) {
		std::cerr << "Errore: " << filename << " non entra in memoria all'indirizzo richiesto" << std::endl;
		return false;
	}

	file.read(reinterpret_cast<char *>(m_flatRam.data() + loadAddress), fileSize);
	return true;
}
