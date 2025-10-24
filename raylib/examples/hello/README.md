# Hello Raylib Example

This example demonstrates running raylib with the software renderer on ESP-IDF using a board-agnostic port layer.

## Features

- **Port Layer Architecture**: Board-agnostic display abstraction (`esp_raylib_port`)
- **Software Renderer**: No OpenGL/GPU required (CPU-based rendering)
- **Dynamic Dimensions**: Automatically queries display size from port
- **Multi-Board Support**: Easy to add new boards via `board_init.c`
- **Automatic Sync**: Semaphore-based synchronization for SPI and DSI panels
- **Smart Byte Swapping**: Automatic for SPI panels, disabled for DSI
- **Simple Animation**: Moving rectangle with centered text and color tests

## Supported Hardware

**Fully Tested & Verified:**
- **ESP32-S3-BOX-3** - 320x240 ILI9341 (SPI) ✅
- **M5Stack Core S3** - 320x240 ILI9342C (SPI) ✅
- **ESP32-P4 Function EV** - 1024x600 EK79007 (MIPI-DSI) ✅

**Configurations Available:**
- **ESP32-C6 DevKit** - Generic display support
- **ESP32-C3 LCDKit** - Development kit

**Note**: To add a new board, just update `main/board_init.c` and create a `sdkconfig.defaults.<board>` file.

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

### How It Works

1. **Board Selection**: Defined in `sdkconfig.defaults.<board>` via `CONFIG_BOARD_*`
2. **Board Init**: `main/board_init.c` creates panel using ESP-BSP for selected board
3. **Port Layer**: `esp_raylib_port` provides board-agnostic display API
4. **Raylib**: Queries dimensions and renders via port layer
5. **Display**: Automatic byte swapping and sync for panel type (SPI/DSI)

**Key Files:**
- `main/Kconfig.projbuild` - Defines board selection options
- `main/board_init.c` - Board-specific ESP-BSP initialization
- `main/hello.c` - Raylib application (queries dimensions dynamically)
- `sdkconfig.defaults.<board>` - Sets `CONFIG_BOARD_<NAME>=y`

**Notes:**
- PSRAM is required and used for framebuffers
- Board selection via Kconfig enables conditional BSP loading
- Multiple boards with same target (e.g., S3) can use different BSPs
- Display dimensions are queried at runtime, not hardcoded

## Expected Behavior

1. **Color Test Sequence**: Full-screen RED, GREEN, BLUE (1 second each)
2. **Animation Loop**: 
   - Alternating background (sky-blue / light-gray every 60 frames)
   - Moving red rectangle across screen
   - Centered text "Hello ESP32!"
   - Corner markers (yellow, orange, purple, pink)

## Performance

- **ESP32-S3-BOX-3** (320x240): ~15-20 FPS
- **M5Stack Core S3** (320x240): ~15-20 FPS
- **ESP32-P4** (1024x600): ~5-10 FPS (larger framebuffer)

## Known Limitations

- Touch input API ready but not connected to raylib
- Audio module not implemented
- 3D models disabled (requires filesystem)
- Large displays (1024x600) render slowly at full resolution
- Text rendering may appear aliased with software backend

## Directory Structure

```
hello/
├── CMakeLists.txt              # Standard ESP-IDF project
├── main/
│   ├── hello.c                # Application (queries dimensions dynamically)
│   ├── board_init.c           # Board-specific BSP initialization
│   ├── board_init.h
│   ├── Kconfig.projbuild      # Board selection menu
│   ├── CMakeLists.txt         # Requires georgik__raylib
│   └── idf_component.yml      # Conditional BSP + component dependencies
├── sdkconfig.defaults         # Base config for all boards
├── sdkconfig.defaults.*       # Board-specific configs
├── support/                   # ESPBrew generated scripts
│   ├── build-esp-box-3.sh
│   ├── flash-esp-box-3.sh
│   └── ...
└── wokwi/                     # Wokwi simulation configs
    ├── esp-box-3/
    ├── m5stack_core_s3/
    └── esp32_p4_function_ev_board/
```

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

### "No board selected" Error
- Ensure `CONFIG_BOARD_*` is set in your sdkconfig
- Check `build/sdkconfig` to verify board config is present
- Rebuild from clean: `rm -rf build && idf.py build`

### Display Not Working
- **SPI panels (S3)**: Check `swap_rgb_bytes = true` in board_init
- **DSI panels (P4)**: Check `swap_rgb_bytes = false` in board_init
- Verify BSP initialization succeeds (check logs)
- Ensure proper DSI bus config for P4 (see board_init.c)

### Colors Wrong
- Toggle `swap_rgb_bytes` in `board_init.c` for your board
- SPI LCDs typically need byte swapping, DSI panels don't

### Stack Overflow
- Ensure PSRAM is enabled: `CONFIG_SPIRAM=y`
- Framebuffers allocated with `buff_psram = true`
- Main task stack: `CONFIG_ESP_MAIN_TASK_STACK_SIZE=65536`
- Raylib task uses 128KB stack (see hello.c)

### Build Errors
- Clean build: `rm -rf build managed_components`
- Reconfigure: `idf.py reconfigure`
- Check ESP-IDF version: 5.5+ required

### P4 Crashes on Init
- Ensure DSI bus config is provided (not NULL)
- Check backlight init: `bsp_display_brightness_init()` first
- Verify panel handle is valid after `bsp_display_new()`
