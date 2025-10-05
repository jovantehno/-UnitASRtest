# UnitASRtest - M5Stack Unit-ASR Testing & Development Tools

Testing and development tools for the **M5Stack Unit-ASR** (CI-03T) offline voice recognition module.

## 🎯 Overview

The M5Stack Unit-ASR is an AI-powered offline speech recognition module based on the **Chipintelli CI-03T** chip. This repository contains Arduino sketches and Python utilities for testing, debugging, and flashing firmware to the Unit-ASR module.

### Features

- ✅ UART communication testing
- ✅ Firmware version detection
- ✅ Voice command recognition demos
- ✅ ESP32 as UART passthrough programmer
- ✅ Python-based firmware flasher (experimental)

## 📦 Hardware Requirements

- **M5Stack Unit-ASR** (CI-03T module)
- **ESP32/ESP32-S3** development board (tested with Waveshare ESP32-S3-Touch-AMOLED-1.8)
- USB cable
- 4 wires for UART connection

## 🔌 Wiring

### Unit-ASR to ESP32 Connection

```
Unit-ASR    →    ESP32
━━━━━━━━━━━━━━━━━━━━━━
TX          →    GPIO44 (RX)
RX          →    GPIO43 (TX)
GND         →    GND
5V/3.3V     →    3.3V
```

**Note:** The Unit-ASR works with 3.3V logic levels. The 5V pin is primarily for the speaker/amplifier.

## 🚀 Quick Start

### 1. Test UART Communication

Upload `arduino/UnitASR_Test/UnitASR_Test.ino` to your ESP32:

```arduino
// Default pins (adjust as needed)
#define ASR_RX_PIN 44  // ESP32 RX ← Unit-ASR TX
#define ASR_TX_PIN 43  // ESP32 TX → Unit-ASR RX
```

**Features:**
- Tests bidirectional UART communication
- Queries firmware status
- Listens for voice commands
- Interactive commands via Serial Monitor (`w`, `q`, `f`)

### 2. Check Firmware Version

Upload `arduino/UnitASR_Firmware_Check/UnitASR_Firmware_Check.ino`:

- Queries firmware version (0x45)
- Tests multiple status commands
- Decodes common voice commands
- Helpful for verifying if firmware is loaded

### 3. Flash New Firmware (Advanced)

#### Option A: Using ESP32 Passthrough + Python (Linux/Mac)

1. Upload `arduino/UART_Passthrough/UART_Passthrough.ino` to ESP32
2. Close Arduino Serial Monitor
3. Run the Python flasher:

```bash
cd python
pip install pyserial
python flash_ci03t.py
```

#### Option B: Using Windows Tool (Recommended)

Use the official Chipintelli flashing tool:
1. Download from M5Stack or firmware generator website
2. Run `PACK_UPDATE_TOOL.exe`
3. Select firmware `.bin` file
4. Flash via UART

## 📡 UART Protocol

The CI-03T uses a simple packet protocol:

```
[0xAA] [0x55] [Command_ID] [0x55] [0xAA]
```

### Common Commands

| Command | Hex | Description |
|---------|-----|-------------|
| Wake word | `0xFF` | "Hi M Five" |
| Turn on | `0x14` | Turn on command |
| Turn off | `0x15` | Turn off command |
| Play | `0x16` | Play audio |
| Pause | `0x17` | Pause audio |
| Next | `0x1B` | Next track |
| Previous | `0x1A` | Previous track |

## 🎤 Voice Commands (Factory Firmware)

Default wake word: **"Hi M Five"**

Supported commands:
- Directions: `up`, `down`, `left`, `right`, `forward`, `backward`
- Controls: `play`, `pause`, `next`, `previous`, `start`, `stop`
- Actions: `turn on`, `turn off`, `open`, `close`
- Numbers: `zero` through `nine`
- Volume: `increase volume`, `decrease volume`

## 🔧 Troubleshooting

### No Voice Recognition

**Symptoms:** UART responds, but voice commands don't work

**Possible causes:**
1. **No firmware loaded** - Unit-ASR echoes all commands back
   - Solution: Flash firmware using Windows tool or custom firmware generator

2. **Device in sleep mode** - Try different wake words
   - Try: "Hi M Five", "Hi ASR", "Hello"

3. **Microphone issue** - Hardware problem
   - Check for physical damage
   - Verify power supply voltage

### UART Communication Issues

**Check:**
- TX/RX wires not swapped
- Baud rate is 115200
- Correct GPIO pins in code
- 3.3V logic levels (ESP32 is NOT 5V tolerant on data pins)

### Firmware Flashing Fails

**Requirements:**
- Windows PC with official `PACK_UPDATE_TOOL.exe`, OR
- Hardware BOOT mode (may require grounding a BOOT pin during power-on)
- Python method is experimental and may not work for all scenarios

## 📚 Resources

- [M5Stack Unit-ASR Official Docs](https://docs.m5stack.com/en/unit/Unit%20ASR)
- [M5Stack Unit-ASR Library](https://github.com/m5stack/M5Unit-ASR)
- [Firmware Customization Guide](https://docs.m5stack.com/en/guide/offline_voice/unit_asr/firmware)
- [Smart Pi Platform](https://www.smartpi.cn/) (Firmware Generator - Chinese)

## 🛠️ Project Structure

```
UnitASRtest/
├── arduino/
│   ├── UnitASR_Test/              # Main UART communication test
│   ├── UnitASR_Firmware_Check/    # Firmware verification tool
│   └── UART_Passthrough/          # ESP32 as flasher passthrough
├── python/
│   └── flash_ci03t.py             # Experimental firmware flasher
├── docs/
│   └── test_factory_firmware.md   # Testing guide
└── README.md
```

## 🤝 Contributing

Contributions welcome! Areas of interest:
- Improved firmware flashing protocol reverse-engineering
- Additional ESP32 board support
- Better hardware BOOT mode documentation
- Custom firmware examples

## ⚠️ Known Issues

1. **Python flasher doesn't work** - The CI-03T likely requires a specific flashing protocol that the Windows tool implements. Consider this experimental.

2. **Wine doesn't run Windows tool** - The Qt-based GUI crashes under Wine. Use native Windows or VM.

3. **No firmware in some units** - Some Unit-ASR modules ship without firmware and echo all UART commands back.

## 📄 License

MIT License - Feel free to use and modify for your projects.

## 🙏 Acknowledgments

- M5Stack for the Unit-ASR hardware
- Chipintelli for the CI-03T voice recognition chip
- Community contributors and testers

---

**Found this useful?** ⭐ Star this repo and share with other makers!

**Have questions?** Open an issue or start a discussion.
