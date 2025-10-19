# 🎓 Recap Implementazione CPU Z80 - Sessione di Apprendimento

## 📚 Cosa Hai Costruito Oggi

Hai creato **da zero** una CPU Z80 funzionante per l'emulatore Pac-Man!

---

## 🏗️ 1. Architettura dei Registri

### Problema Iniziale
Come rappresentare registri che possono essere usati sia come 8-bit che come 16-bit?

### Soluzione Implementata
```cpp
union RegisterPair {
    uint16_t pair;  // Accesso a 16-bit
    struct {
        uint8_t low;   // Byte basso
        uint8_t high;  // Byte alto
    };
};
```

### Perché Funziona
- **Zero overhead**: Nessuna conversione runtime
- **Type-safe**: Il compilatore garantisce la correttezza
- **Natural access**: `BC.pair` per 16-bit, `BC.high`/`BC.low` per 8-bit
- **Endianness-aware**: Funziona su architetture little-endian (x86/x64)

### Concetti Appresi
- Le `union` permettono di "reinterpretare" gli stessi byte in modi diversi
- L'endianness conta quando lavori a basso livello
- Il layout in memoria influenza le performance

---

## 🚦 2. Flag Register e Bit Manipulation

### Problema
Come rappresentare 6 flag booleani in modo efficiente e veloce?

### Soluzione
```cpp
uint8_t F;  // Un solo byte per tutti i flag

const uint8_t FLAG_C  = 0x01;  // Bit 0
const uint8_t FLAG_N  = 0x02;  // Bit 1
const uint8_t FLAG_PV = 0x04;  // Bit 2
const uint8_t FLAG_H  = 0x10;  // Bit 4
const uint8_t FLAG_Z  = 0x40;  // Bit 6
const uint8_t FLAG_S  = 0x80;  // Bit 7

void SetFlag(uint8_t flag, bool value) {
    if (value)
        F |= flag;   // Set: OR
    else
        F &= ~flag;  // Clear: AND NOT
}

bool GetFlag(uint8_t flag) const {
    return (F & flag) != 0;  // Test: AND
}
```

### Le 4 Operazioni Fondamentali
```cpp
// 1. SET bit
value |= mask;

// 2. CLEAR bit
value &= ~mask;

// 3. TOGGLE bit
value ^= mask;

// 4. TEST bit
(value & mask) != 0
```

### Concetti Appresi
- Il bit manipulation è essenziale per emulatori e sistemi embedded
- Ogni operazione ha un costo: OR/AND sono velocissime
- I flag permettono di rappresentare multiple condizioni in poco spazio

---

## 🔄 3. Fetch-Decode-Execute Cycle

### Il Cuore della CPU
```cpp
int Z80::Step() {
    // FETCH
    uint8_t opcode = m_memory->Read(PC++);
    
    // DECODE & EXECUTE
    (this->*m_opcodeTable[opcode])();
    
    // TIMING
    m_totalCycles += m_cyclesLastInstruction;
    return m_cyclesLastInstruction;
}
```

### Pattern: Opcode Table Dispatch
```cpp
// Invece di un gigantesco switch...
using OpcodeFunction = void (Z80::*)();
OpcodeFunction m_opcodeTable[256];

// Inizializzazione
m_opcodeTable[0x3E] = &Z80::OP_LD_A_n;

// Chiamata
(this->*m_opcodeTable[opcode])();  // Velocissimo!
```

### Concetti Appresi
- Le CPU sono **loop infiniti** di fetch-decode-execute
- I **function pointers** sono più veloci di switch giganti
- La sintassi dei **member function pointers** in C++
- Il **timing** è importante per emulazione accurata

---

## 🎯 4. Helper Functions Pattern

### Problema
Come evitare di copiare lo stesso codice 7 volte?

### Anti-Pattern (BAD)
```cpp
void OP_INC_A() { /* 10 righe di codice */ }
void OP_INC_B() { /* stesse 10 righe */ }
void OP_INC_C() { /* stesse 10 righe */ }
// 😱 Copy-paste nightmare!
```

### Pattern (GOOD)
```cpp
// 1 helper function
void INC_r(uint8_t& reg) { /* 10 righe */ }

// 7 opcode handlers (one-liner)
void OP_INC_A() { INC_r(A); }
void OP_INC_B() { INC_r(BC.high); }
void OP_INC_C() { INC_r(BC.low); }
// 👏 DRY principle!
```

### Risultato
- **1 helper function = 7 opcode**
- Se trovi un bug, lo correggi **una volta sola**
- Codice più leggibile e manutenibile

### Concetti Appresi
- **DRY** (Don't Repeat Yourself) non è solo teoria
- **Reference parameters** (`uint8_t&`) per modificare variabili
- Separazione tra **logica** (helper) e **dispatch** (opcode handler)

---

## 🧮 5. Carry vs Overflow - Il Concetto Chiave

### La Domanda Cruciale
> "Essendo i registri a 8 bit nello Z80 ci può essere carry o c'è sempre overflow?"

### Risposta
**ENTRAMBI, INDIPENDENTEMENTE!**

```
         Hardware (Z80)          Software (Programmatore)
              ↓                           ↓
    Calcola TUTTI i flag         Guarda solo quello che serve
              ↓                           ↓
        C + P/V settati      Interpreta come signed/unsigned
```

### Tabella della Verità

| Esempio | A | value | result | Carry? | Overflow? |
|---------|---|-------|--------|--------|-----------|
| 50+30 | 50 | 30 | 80 | ❌ | ❌ |
| 200+100 | 200 | 100 | 44 | ✅ | ❌ |
| 100+50 | 100 | 50 | 150 | ❌ | ✅ |
| 150+150 | 150 | 150 | 44 | ✅ | ✅ |

### Concetti Appresi
- La CPU **non sa** se i numeri sono signed o unsigned
- Il **software decide** quale flag guardare
- **Carry** → unsigned (0-255)
- **Overflow** → signed (-128 a +127)
- Questo vale per **tutte le CPU** moderne!

---

## 🎨 6. Calcolo dell'Overflow (XOR Magic)

### Formule
```cpp
// ADD
bool overflow = ((oldA ^ result) & (value ^ result) & 0x80) != 0;

// SUB
bool overflow = ((oldA ^ value) & (oldA ^ result) & 0x80) != 0;
```

### Cosa Controlla
1. **Gli operandi hanno cambiato segno?**
2. **Il risultato ha segno inaspettato?**
3. **Il bit 7 è coinvolto?**

### Esempi
```
ADD: 100 + 50 = 150
     positivo + positivo = negativo → OVERFLOW!

SUB: 100 - (-50) = 150  
     positivo - negativo = negativo → OVERFLOW!
```

### Concetti Appresi
- Il calcolo dell'overflow usa **XOR bitwise** per confrontare segni
- È un **pattern matematico** usato in tutte le CPU
- Capire il **perché** è più importante del **come**

---

## 💪 7. Design Patterns Applicati

### Pattern 1: Dependency Injection
```cpp
Z80::Z80(MemoryBus* memory) : m_memory(memory) {
    // La CPU non crea il MemoryBus, lo riceve
}
```
**Vantaggio:** Testabilità, flessibilità, separazione

### Pattern 2: Template Method (via Function Pointers)
```cpp
// Algoritmo fisso
int Step() {
    fetch();
    decode_and_execute();  // ← Varia per ogni opcode
    update_timing();
}
```

### Pattern 3: Strategy (Helper Functions)
```cpp
// Stessa interfaccia, comportamento variabile
void INC_r(uint8_t& reg);   // Strategia: incremento
void DEC_r(uint8_t& reg);   // Strategia: decremento
```

---

## 📊 Statistiche Implementazione

### Opcode Implementati
- **1x** NOP (0x00)
- **7x** INC (0x04, 0x0C, 0x14, 0x1C, 0x24, 0x2C, 0x3C)
- **7x** DEC (0x05, 0x0D, 0x15, 0x1D, 0x25, 0x2D, 0x3D)
- **7x** LD r,n (0x06, 0x0E, 0x16, 0x1E, 0x26, 0x2E, 0x3E)
- **7x** ADD A,r (0x80-0x85, 0x87)
- **7x** SUB A,r (0x90-0x95, 0x97)
- **1x** LD B,C (0x41)

**Totale: 37 opcode / 256 (14.5%)**

### Helper Functions Create
- `INC_r()` - Incremento con flag
- `DEC_r()` - Decremento con flag
- `LD_r_n()` - Load immediate
- `LD_r_r()` - Load register to register
- `ADD_A_r()` - Addizione con carry e overflow
- `SUB_A_r()` - Sottrazione con borrow e overflow

---

## 🎓 Concetti C++ Avanzati Appresi

1. ✅ **Union** per rappresentazione dati multi-formato
2. ✅ **Bit manipulation** avanzata (flag, half-carry, overflow)
3. ✅ **Member function pointers** e tabelle di dispatch
4. ✅ **Reference parameters** (`uint8_t&`) per modificare registri
5. ✅ **Helper functions pattern** per code reuse
6. ✅ **Exception handling** con `std::invalid_argument`
7. ✅ **Member initializer list**
8. ✅ **Naming conventions** (m_ prefix)

---

## 🎯 Domande di Auto-Valutazione

### Domanda 1
Perché `RegisterPair` è una `union` e non una `struct` con due membri separati?

### Domanda 2
Cosa succederebbe se `OP_LD_A_n` non incrementasse PC dopo aver letto l'operando?

### Domanda 3
In `ADD A, 0xFF`, quali flag saranno settati se A = 0x01?

### Domanda 4
Perché `using OpcodeFunction = void (Z80::*)()` invece di `void (*)()` ?

### Domanda 5
Come faresti a implementare `LD r,r'` completo (tutti i 64 opcode 0x40-0x7F) in modo efficiente?

---

## 🏆 Progresso Attuale

```
[████████████░░░░░░░░] 60% Completamento CPU Z80

✅ Architettura registri
✅ Flag system completo  
✅ Fetch-Decode-Execute
✅ 37 opcode funzionanti
✅ Helper functions pattern
⬜ LD r,r completi (64 opcode)
⬜ Operazioni memoria
⬜ Jump/Call/Ret
⬜ Stack operations
⬜ Interrupt handling
```

---

## 🚀 Prossimi Passi

1. **Implementare LD r,r completi** (0x40-0x7F) - c'è un pattern matematico!
2. **Operazioni memoria**: `LD A,(HL)`, `LD (HL),A`, ecc.
3. **Jump condizionali**: `JP Z,nn`, `JR NZ,e`, ecc.
4. **Stack e subroutine**: `PUSH`, `POP`, `CALL`, `RET`
5. **Interrupt handling**: `IM`, `EI`, `DI`, `RETI`

---

## 📖 Approfondimenti Opzionali

- **Endianness**: Cosa succederebbe su architetture big-endian?
- **Cache performance**: Perché la opcode table è veloce?
- **Two's complement**: Matematica dietro i numeri con segno
- **Virtual functions**: Differenza con function pointers
- **SIMD**: Come le CPU moderne fanno operazioni parallele

---

## 💡 Lezioni Chiave

### 1. La CPU non distingue signed/unsigned
L'hardware calcola tutti i flag meccanicamente. Il software decide quale guardare.

### 2. DRY è pratico, non teorico
Una helper function elimina duplicazione e semplifica manutenzione.

### 3. I bit sono il linguaggio delle CPU
Padroneggiare OR, AND, XOR, NOT è essenziale per programmazione low-level.

### 4. L'architettura guida il design
La struttura della CPU Z80 ha guidato naturalmente verso registri pubblici e helper functions.

### 5. Il debugging viene dal ragionamento
Trovare il bug della ROM è venuto dal ragionare sulla memoria, non dal debugger.

---

## 🌟 Punti di Forza Dimostrati

- ✅ Domande intelligenti invece di accettazione passiva
- ✅ Bug trovati autonomamente (problema ROM)
- ✅ Comprensione dei concetti oltre il codice
- ✅ Sfida delle spiegazioni quando non chiare
- ✅ Implementazione corretta al primo tentativo (con piccole correzioni)

---

*Documento generato il: 2025-01-18*
*Progetto: PacmanEmulator - CPU Z80 Implementation*
*Linguaggio: C++17 con Visual Studio 2022*
*Librerie: SFML 3.0*