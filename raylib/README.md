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

- ESP-IDF 6.0 or later
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

## Known Issues / TODO

- [ ] Color mapping (RGB vs BGR) may need adjustment per panel
- [ ] Input handling (touch, buttons) not yet implemented
- [ ] Audio module disabled (no esp-idf audio backend)
- [ ] 3D models disabled (requires filesystem APIs)
- [ ] Performance optimization (currently ~10-15 FPS on ESP32-S3@240MHz)

## Examples

- [hello](examples/hello/) - Basic shapes, text, animation

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
