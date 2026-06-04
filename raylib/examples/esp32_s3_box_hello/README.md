# Raylib Hello Example for ESP32-S3-BOX

A minimal Raylib graphics example demonstrating 2D rendering on ESP32-S3-BOX hardware.

## Hardware

**Target Board**: ESP32-S3-BOX
- Display: 320x240 ILI9341 LCD (SPI)
- Touch: TT21100 resistive touch controller
- SoC: ESP32-S3 (dual-core Xtensa, 512KB SRAM, 8MB PSRAM)

## Requirements

- ESP-IDF v6.1 or later (release/v6.1 branch)
- CMake 3.22 or later
- Xtensa ESP32-S3 toolchain

## Architecture

This example uses a minimal architecture without board support packages:

- **Direct LCD configuration** via ESP-IDF SPI LCD driver
- **Custom display callbacks** for framebuffer flushing
- **Software renderer** (GRAPHICS_API_OPENGL_SOFTWARE)
- **Reduced component set** for faster builds

### Component Structure

```
main/
  - Main application (Raylib initialization, drawing)
raylib/
  - Core Raylib library (rcore, rtext, rtextures, rshapes)
  - ESP-IDF platform layer (rcore_esp_idf.c)
esp_raylib_port/
  - Display/touch abstraction layer
```

## Building

```bash
# Set ESP-IDF environment
. $HOME/.espressif/master/esp-idf/export.sh

# Configure and build
idf.py build

# Flash to device
idf.py -p /dev/ttyUSB0 flash

# Monitor output
idf.py -p /dev/ttyUSB0 monitor
```

## Build Configuration

The project uses a minimal component set for faster compilation:

- **CMake 3.22** required
- **COMPONENTS=main** restricts build to required components only
- **Weak symbol stubs** for POSIX functions (access, mkdir)

## Memory Usage

- Binary size: ~302KB
- Flash usage: 29% of 1MB partition
- PSRAM required: 8MB external

## Troubleshooting

**Linker errors regarding `access` or `mkdir`**: Ensure ESP-IDF v6.1+ is being used. Weak symbol stubs are provided in `rcore_esp_idf.c`.

**Display not updating**: Verify SPI GPIO configuration matches hardware. Check `raylib_esp_set_display_callbacks()` is called before `InitWindow()`.

**Touch input not working**: Confirm TT21100 driver is properly initialized and touch IRQ is configured.
