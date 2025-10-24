# Raylib for ESP-IDF

ESP-IDF component wrapper for [raylib](https://www.raylib.com/) using the **software renderer** (rlgl OpenGL 1.1 software backend).

This component enables running raylib on ESP32 devices without GPU/OpenGL support by using the CPU-based software renderer merged in [raylib PR #4832](https://github.com/raysan5/raylib/pull/4832).

## Architecture

This implementation uses a **port layer architecture** for board independence:

```
Application (hello.c)
    ↓
board_init.c (creates panel via ESP-BSP)
    ↓
esp_raylib_port (board-agnostic display API)
    ↓
raylib (rcore_esp_idf.c platform backend)
    ↓
esp_lcd (ESP-IDF display driver)
```

**Key benefits:**
- Stable API between raylib and display hardware
- Deterministic builds (board selection via Kconfig)
- Easy to add new boards (just implement board_init)
- Supports both SPI (S3) and DSI (P4) panels

## Features

- ✅ Software rendering (no OpenGL hardware required)
- ✅ RGB565 framebuffer support for ESP LCD panels
- ✅ PSRAM allocation for framebuffers (heap-based, not stack)
- ✅ Automatic byte swapping for SPI panels
- ✅ LCD synchronization with semaphores (SPI and DSI)
- ✅ Dynamic display dimensions (queries from port)
- ✅ Chunked transfers for SPI, direct for DSI
- ✅ Compatible with esp-bsp noglib components
- ✅ 2D graphics: shapes, textures, text rendering

## Supported Boards

**Fully tested and verified:**
- **ESP32-S3-BOX-3** (320x240 ILI9341, SPI) - ✅ Working
- **M5Stack Core S3** (320x240 ILI9342C, SPI) - ✅ Working
- **ESP32-P4 Function EV Board** (1024x600 EK79007, MIPI-DSI) - ✅ Working

**Easy to add:** Any board with ESP-BSP noglib support. Just add to `board_init.c`.

## Requirements

- ESP-IDF 5.5 or later (tested with latest master)
- ESP32-S3 or similar with PSRAM (recommended for framebuffer)
- Board Support Package (BSP) for your target board

## Installation

### Using ESP Component Registry (coming soon)
```yaml
dependencies:
  georgik/raylib: "*"
```

### Local Development
Clone this repository into your project's `components/` directory or use as a git submodule.

## Quick Start

See the [hello example](examples/hello/README.md) for a complete working application.

### Building the Example

```bash
cd examples/hello
idf.py set-target esp32s3
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.defaults.esp-box-3" reconfigure build flash monitor
```

## Component Structure

```
raylib/
├── CMakeLists.txt              # ESP-IDF component build configuration
├── idf_component.yml           # Component metadata
├── esp_raylib_port/              # Port layer
│   ├── include/esp_raylib_port.h  # Port API
│   ├── src/esp_raylib_port.c      # Implementation
│   ├── CMakeLists.txt
│   └── idf_component.yml
├── include/                    # Wrapper headers (stubs, overrides)
│   ├── dirent.h               # Stub for missing POSIX functions
│   └── rlsw_esp_idf.h         # Software renderer config (PSRAM, RGB format)
├── src/
│   └── platforms/
│       └── rcore_esp_idf.c    # ESP-IDF platform backend
├── raylib/                     # Git submodule: official raylib
│   └── raylib/                 # (upstream at master branch)
└── examples/
    └── hello/                  # Demo application
        └── main/
            ├── hello.c         # Application code
            ├── board_init.c    # Board-specific init
            ├── board_init.h
            └── Kconfig.projbuild  # Board selection
```

## Configuration

The component is configured via CMake definitions in `CMakeLists.txt`:

- `GRAPHICS_API_OPENGL_11_SOFTWARE` - Enable software renderer
- `SW_COLOR_BUFFER_BITS=16` - RGB565 format
- `SW_MALLOC` / `SW_FREE` - Allocate from PSRAM
- Disabled modules: raudio, rmodels, compression, automation

## Port Layer API (`esp_raylib_port`)

The port layer provides a stable, board-agnostic API:

**Initialization:**
```c
ray_port_cfg_t cfg = {
    .buff_psram = true,
    .swap_rgb_bytes = true,  // For SPI panels
    .hres = 320,
    .vres = 240,
    .perf_stats = true,
};
ray_port_init(&cfg);

ray_port_display_cfg_t disp = {
    .panel = panel_handle,
    .io = io_handle,
    .hres = 320,
    .vres = 240,
    .dma_capable = true,
};
ray_port_add_display(&disp);
```

**Key features:**
- RGB565 framebuffer flushing with chunking
- Automatic byte swapping for SPI panels
- LCD synchronization via callbacks and semaphores
- Thread-safe operations
- Performance statistics

### Color Mapping Implementation

The framebuffer uses RGB565 format (5 bits red, 6 bits green, 5 bits blue). The implementation handles color format correctly through:

1. **Direct Framebuffer Access**: Uses `swGetColorBuffer()` to access the software renderer's internal RGB565 buffer directly, avoiding lossy format conversions
2. **Byte-Order Conversion**: SPI LCD panels (like ILI9341 on ESP32-S3-BOX-3) expect big-endian RGB565 format, so bytes are swapped before transmission:
   ```c
   framebuffer[i] = (pixel >> 8) | (pixel << 8);  // Little-endian to big-endian
   ```

**Important**: Earlier implementations used `rlCopyFramebuffer()` which performed RGB565 → RGBA8888 → RGB565 conversion, corrupting colors. The current implementation uses direct memory copy to preserve color accuracy.

## Known Issues / TODO

- [x] ~~Color mapping~~ - Fixed: Automatic byte swapping per panel type
- [x] ~~Display synchronization~~ - Fixed: Semaphore-based sync for SPI and DSI
- [x] ~~Multi-board support~~ - Fixed: Board selection via Kconfig
- [ ] Touch input API ready but not connected to raylib
- [ ] Audio module disabled (no esp-idf audio backend)
- [ ] 3D models disabled (requires filesystem APIs)
- [ ] Performance: ~15-20 FPS on 320x240, slower on 1024x600

## Examples

- [hello](examples/hello/) - Basic shapes, text, animation

## CI/CD

This project includes GitHub Actions workflows for automated building and testing:

- **Build**: Automatically builds examples for ESP32-S3-BOX-3, M5Stack Core S3, and ESP32-P4 Function EV Board
- **Test**: Runs Wokwi simulations and captures screenshots

See [.github/README.md](.github/README.md) for detailed CI/CD documentation.

## Contributing

Contributions welcome! Especially:
- Additional board support
- Performance optimizations
- Input handling (touch, buttons)
- Color format fixes

## License

This wrapper component: zlib/libpng (matching raylib)

Raylib: zlib/libpng - see [raylib/raylib/LICENSE](raylib/raylib/LICENSE)

## Credits

- [raylib](https://www.raylib.com/) by Ramon Santamaria (@raysan5)
- Software renderer (rlsw) by Le Juez Victor (@Bigfoot71)
- ESP-IDF platform integration by this component
