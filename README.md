# Pac-Man Emulator

Emulatore di Pac-Man scritto in C++ con SFML 3.0.

## 🎮 Stato del Progetto

### ✅ Componenti Completati

#### Memory Bus
- Memory mapping completo (ROM, VRAM, Color RAM, RAM, Sprite RAM)
- RAM mirroring (0x4800-0x4FFF)
- Metodi Read/Write funzionanti

#### CPU Z80
- **101/256 opcode implementati (39.5%)**
- Fetch-Decode-Execute cycle completo
- Flag register con gestione corretta di tutti i flag
- Opcode table con function pointers per dispatch veloce
- Register mapping array per lookup O(1)

##### Categorie Opcode Implementate:
- ✅ **NOP, HALT** (2 opcode)
- ✅ **INC/DEC r** (14 opcode) - incremento/decremento 8-bit con flag
- ✅ **LD r,n** (7 opcode) - caricamento immediato
- ✅ **LD r,r'** (64 opcode) - copia registri con accesso memoria (HL)
- ✅ **ADD A,r** (7 opcode) - addizione con carry e overflow
- ✅ **SUB A,r** (7 opcode) - sottrazione con borrow e overflow

##### Features CPU:
- ✅ Architettura registri completa (A, B, C, D, E, H, L, F)
- ✅ Shadow registers (A', F', BC', DE', HL')
- ✅ Registri speciali (PC, SP, IX, IY, I, R)
- ✅ Flag register (Z, S, H, PV, N, C)
- ✅ Gestione corretta carry vs overflow (unsigned vs signed)
- ✅ Half-carry calculation
- ✅ Cycle-accurate timing
- ✅ Accesso memoria tramite (HL)

### 🧪 Testing
- ✅ 7/7 test CPU passati
- ✅ Tutti i flag verificati
- ✅ Timing accurato

### ⏳ Da Implementare

#### CPU Z80
- ⬜ Operazioni logiche (AND, OR, XOR, CP)
- ⬜ Operazioni 16-bit (LD rr,nn, INC rr, DEC rr)
- ⬜ Jump e branch (JP, JR, CALL, RET)
- ⬜ Stack operations (PUSH, POP)
- ⬜ Rotate e shift (RLC, RRC, RL, RR, SLA, SRA, SRL)
- ⬜ Bit operations (BIT, SET, RES)
- ⬜ Interrupt handling (IM, EI, DI, RETI)
- ⬜ Opcode estesi (CB, ED, DD, FD prefix)

#### Altri Componenti
- ⬜ Video rendering
- ⬜ Input handling
- ⬜ Audio
- ⬜ Caricamento ROM Pac-Man

## 🛠️ Tecnologie

- **Linguaggio**: C++17
- **IDE**: Visual Studio 2022
- **Librerie**: SFML 3.0
- **Build System**: Visual Studio Project

## 📚 Architettura

### Design Patterns Utilizzati
- **Dependency Injection**: CPU riceve MemoryBus via costruttore
- **Strategy Pattern**: Helper functions per riuso codice
- **Table-Driven Dispatch**: Opcode table per performance

### Ottimizzazioni
- Register mapping array per lookup O(1)
- Function pointers per evitare switch giganti
- Helper functions per eliminare duplicazione codice
- Union per accesso 8/16-bit senza overhead

## 📖 Documentazione Apprendimento

Vedi `docs/Z80_Learning_Recap.md` per dettagli su:
- Architettura registri Z80
- Flag register e bit manipulation
- Fetch-Decode-Execute cycle
- Calcolo carry vs overflow
- Pattern matematici negli opcode

## 🚀 Build

1. Apri il progetto in Visual Studio 2022
2. Configura SFML 3.0 nelle impostazioni del progetto
3. Build e Run

## 📈 Progresso
```
CPU Z80: [████████░░░░░░░] 40%
Memory:  [██████████████] 100%
Video:   [░░░░░░░░░░░░░░] 0%
Audio:   [░░░░░░░░░░░░░░] 0%
Input:   [░░░░░░░░░░░░░░] 0%
```

## 📝 License

Progetto educativo per apprendimento programmazione C++ avanzata.