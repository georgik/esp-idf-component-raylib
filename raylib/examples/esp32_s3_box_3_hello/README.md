# Raylib Hello Example for ESP32-S3-BOX-3

A Raylib graphics example demonstrating 2D rendering on ESP32-S3-BOX-3 hardware with full sensor integration.

## Hardware

**Target Board**: ESP32-S3-BOX-3
- Display: 320x240 ILI9341 LCD (SPI)
- Touch: GT911 capacitive touch controller
- Sensors: ICM-42670 (6-axis IMU), AHT30 (temperature/humidity)
- SoC: ESP32-S3 (dual-core Xtensa, 512KB SRAM, 8MB PSRAM)
- Audio: ES8311 codec, built-in microphone and speaker

## Requirements

- ESP-IDF v6.1 or later (release/v6.1 branch)
- CMake 3.22 or later
- Xtensa ESP32-S3 toolchain

## Architecture

This example uses the ESP-BOX-3 board support package (BSP) via managed components:

- **ESP-BOX-3 BSP** for hardware abstraction
- **Managed components** for drivers and sensors
- **Software renderer** (GRAPHICS_API_OPENGL_SOFTWARE)
- **Sensor hub** for IMU and environmental sensors

### Component Structure

```
main/
  - Main application (Raylib initialization, drawing)
raylib/
  - Core Raylib library (rcore, rtext, rtextures, rshapes)
  - ESP-IDF platform layer (rcore_esp_idf.c)
esp_raylib_port/
  - Display/touch abstraction layer
espressif__esp-box-3_noglib/
  - Board support package (managed component)
espressif__esp_lcd_touch_gt911/
  - GT911 touch driver (managed component)
espressif__icm42670/
  - 6-axis IMU driver (managed component)
espressif__aht30/
  - Temperature/humidity sensor (managed component)
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
- **Weak symbol stubs** for POSIX functions (access, mkdir) - overridden by ESP-IDF VFS when present
- **Managed components** automatically fetched during CMake configuration

## Memory Usage

- Binary size: ~274KB
- Flash usage: 26% of 1MB partition
- PSRAM required: 8MB external

## Managed Components

This project uses ESP Component Manager for the following components:

| Component | Purpose |
|-----------|---------|
| espressif__esp-box-3_noglib | Board support package |
| espressif__esp_lcd_touch_gt911 | Capacitive touch driver |
| espressif__icm42670 | 6-axis IMU (accelerometer/gyroscope) |
| espressif__aht30 | Temperature and humidity sensor |
| espressif__esp_lcd_ili9341 | LCD driver |
| espressif__button | Button debouncing |
| espressif__esp_codec_dev | Audio codec abstraction |

## Troubleshooting

**Missing managed components**: Run `idf.py fullclean` to re-fetch components.

**Linker errors regarding `access` or `mkdir`**: Ensure ESP-IDF v6.1+ is being used. Weak symbol stubs in `rcore_esp_idf.c` do not conflict with ESP-IDF VFS.

**Sensor data not available**: Verify I2C bus configuration and sensor initialization in BSP.

**Touch input not working**: Check GT911 interrupt pin configuration and I2C communication.
