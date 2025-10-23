# Hello Raylib Example

This example demonstrates running raylib with the software renderer on ESP-IDF.

Features:
- Software renderer (no OpenGL) via raylib rlgl OpenGL 1.1 software backend
- Double-buffered SPI transfers in chunks using esp-bsp pattern
- Simple animation and centered text
- Supports multiple ESP32 boards (ESP-BOX-3, M5Stack Core S3, ESP32-P4, etc.)

## Supported Hardware
- **ESP32-S3-BOX-3** - 320x240 ILI9341 (recommended)
- **M5Stack Core S3** - 320x240 ILI9341
- **ESP32-P4 Function EV** - 1280x800 MIPI-DSI
- **ESP32-C6 DevKit** - Generic display support
- **ESP32-C3 LCDKit** - Development kit

## Build and Flash

### Method 1: Using SDKCONFIG_DEFAULTS (Explicit)
```bash
# For ESP-BOX-3
idf.py set-target esp32s3
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.defaults.esp-box-3" reconfigure build flash monitor

# For M5Stack Core S3
idf.py set-target esp32s3
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.defaults.m5stack_core_s3" reconfigure build flash monitor

# For ESP32-P4 Function EV
idf.py set-target esp32p4
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.defaults.esp32_p4_function_ev_board" reconfigure build flash monitor
```

### Method 2: Using Environment Variable
```bash
# Set once
export SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.defaults.esp-box-3"
idf.py set-target esp32s3
idf.py reconfigure build flash monitor
```

### Method 3: Using ESPBrew (Recommended for Multi-Board)
```bash
# Interactive TUI - select boards and build
espbrew .

# Or CLI mode - builds all boards automatically
espbrew --cli-only .

# Generated helper scripts
./build_esp-box-3.sh
./flash_esp-box-3.sh
```

**ESPBrew** automatically detects all `sdkconfig.defaults.*` files and generates build/flash scripts for each board. Perfect for testing across multiple boards or CI/CD.

Install ESPBrew: https://github.com/georgik/espbrew

### Notes
- PSRAM is required and used for renderer buffers
- Board-specific configurations are in `sdkconfig.defaults.<board>` files
- Each board configuration sets appropriate target, BSP, and optimization flags
- **Board selection is done via Kconfig**: Each `sdkconfig.defaults.<board>` includes `CONFIG_RAYLIB_BOARD_*` which determines which BSP component is loaded
- This allows multiple boards with the same target (e.g., ESP-BOX-3 and M5Stack Core S3 both use ESP32-S3) to use different BSPs

## Expected behavior
- On startup, color test sequence: full-screen RED, GREEN, BLUE (1s each)
- Then alternating background (sky-blue / light-gray), moving red square, and centered text "Hello ESP32!"

## Known limitations (WIP)
- Color mapping still needs verification (RGB/BGR ordering)
- Text rendering may appear less crisp with software backend
- No input handling yet; the app runs continuously

## Directory Structure
- `main/hello.c` - Example application
- `sdkconfig.defaults` - Base configuration for all boards
- `sdkconfig.defaults.<board>` - Board-specific configurations
- `wokwi/<board>/` - Wokwi simulation files per board

## Wokwi Simulation

This example includes Wokwi simulation configurations for multiple boards.

### Available Boards
- `wokwi/esp-box-3/` - ESP32-S3-BOX-3
- `wokwi/m5stack_core_s3/` - M5Stack Core S3
- `wokwi/esp32_c6_devkit/` - ESP32-C6 DevKit
- `wokwi/esp32_p4_function_ev_board/` - ESP32-P4 Function EV

### Requirements
- [wokwi-cli](https://docs.wokwi.com/wokwi-ci/getting-started) installed
- Build project first using one of the methods above

### Run Simulation
```bash
# ESP-BOX-3 simulation
wokwi-cli --diagram-file wokwi/esp-box-3/diagram.json

# M5Stack Core S3 simulation
wokwi-cli --diagram-file wokwi/m5stack_core_s3/diagram.json

# Take screenshot (5 seconds after startup)
wokwi-cli --diagram-file wokwi/esp-box-3/diagram.json --screenshot-part esp --screenshot-time 5000 --screenshot-file screenshot.png --timeout 6000 --timeout-exit-code 0
```

## Troubleshooting
- If you see SPI queue errors or partial updates, ensure chunked transfers are used and PSRAM is enabled.
- If stack overflows occur, ensure the app runs in a dedicated task with a larger stack (example uses 128KB).
