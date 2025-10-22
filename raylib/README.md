# Raylib for ESP-IDF

ESP-IDF component wrapper for [raylib](https://www.raylib.com/) using the **software renderer** (rlgl OpenGL 1.1 software backend).

This component enables running raylib on ESP32 devices without GPU/OpenGL support by using the CPU-based software renderer merged in [raylib PR #4832](https://github.com/raysan5/raylib/pull/4832).

## Features

- ✅ Software rendering (no OpenGL hardware required)
- ✅ RGB565 framebuffer support for ESP LCD panels
- ✅ PSRAM allocation for framebuffers (heap-based, not stack)
- ✅ Chunked SPI transfers following esp-bsp patterns
- ✅ Compatible with esp-bsp noglib components
- ✅ 2D graphics: shapes, textures, text rendering

## Supported Boards

Currently tested on:
- **ESP32-S3-BOX-3** (320x240 ILI9341 display)

Should work on other ESP32 boards with SPI LCD panels (may require BSP dependency updates).

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
idf.py @boards/esp-box-3.cfg build flash monitor
```

## Component Structure

```
raylib/
├── CMakeLists.txt              # ESP-IDF component build configuration
├── idf_component.yml           # Component metadata
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
```

## Configuration

The component is configured via CMake definitions in `CMakeLists.txt`:

- `GRAPHICS_API_OPENGL_11_SOFTWARE` - Enable software renderer
- `SW_COLOR_BUFFER_BITS=16` - RGB565 format
- `SW_MALLOC` / `SW_FREE` - Allocate from PSRAM
- Disabled modules: raudio, rmodels, compression, automation

## Platform Backend (`rcore_esp_idf.c`)

Key functions implemented:
- `InitPlatform()` - Initialize BSP display
- `WindowShouldClose()` - Returns `false` (run indefinitely on embedded)
- `SwapScreenBuffer()` - Copy software-rendered framebuffer to LCD via `esp_lcd_panel_draw_bitmap`
- `CreateWindowFramebuffer()` - Allocate RGB565 buffer in PSRAM

### Color Mapping Implementation

The framebuffer uses RGB565 format (5 bits red, 6 bits green, 5 bits blue). The implementation handles color format correctly through:

1. **Direct Framebuffer Access**: Uses `swGetColorBuffer()` to access the software renderer's internal RGB565 buffer directly, avoiding lossy format conversions
2. **Byte-Order Conversion**: SPI LCD panels (like ILI9341 on ESP32-S3-BOX-3) expect big-endian RGB565 format, so bytes are swapped before transmission:
   ```c
   framebuffer[i] = (pixel >> 8) | (pixel << 8);  // Little-endian to big-endian
   ```

**Important**: Earlier implementations used `rlCopyFramebuffer()` which performed RGB565 → RGBA8888 → RGB565 conversion, corrupting colors. The current implementation uses direct memory copy to preserve color accuracy.

## Known Issues / TODO

- [x] ~~Color mapping~~ - Fixed: Direct RGB565 framebuffer access with byte-swapping for SPI LCDs
- [ ] Input handling (touch, buttons) not yet implemented
- [ ] Audio module disabled (no esp-idf audio backend)
- [ ] 3D models disabled (requires filesystem APIs)
- [ ] Performance optimization (currently ~10-15 FPS on ESP32-S3@240MHz)
- [ ] Color format may need adjustment for non-SPI panels (e.g., parallel RGB, MIPI-DSI)

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
