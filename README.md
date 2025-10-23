# ESP-IDF Component: Raylib

ESP-IDF wrapper component for [raylib](https://www.raylib.com/) - a simple and easy-to-use library to enjoy videogames programming.

This component enables running raylib on ESP32 microcontrollers using the **CPU-based software renderer** (no GPU required), making it possible to create graphics applications on embedded devices with LCD displays.

## 🚀 Quick Start

```bash
cd raylib/examples/hello

# Method 1: Explicit SDKCONFIG_DEFAULTS
idf.py set-target esp32s3
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.defaults.esp-box-3" reconfigure build flash monitor

# Method 2: Using ESPBrew (recommended for multi-board development)
espbrew --cli-only .
```

## 📦 Repository Structure

```
esp-idf-component-raylib/
├── README.md                   # This file
└── raylib/                     # ESP-IDF component
    ├── README.md              # Component documentation
    ├── CMakeLists.txt         # Component build configuration
    ├── idf_component.yml      # Component metadata
    ├── include/               # Wrapper headers
    ├── src/                   # ESP-IDF platform backend
    ├── raylib/                # Submodule: official raylib (master)
    └── examples/
        └── hello/             # Demo application
            └── README.md      # Example documentation
```

## 📚 Documentation

- **[Component Documentation](raylib/README.md)** - Architecture, configuration, and API
- **[Hello Example](raylib/examples/hello/README.md)** - Complete working example with build instructions

## ✨ Features

- ✅ **Software Rendering** - No OpenGL/GPU required, runs entirely on CPU
- ✅ **RGB565 Framebuffer** - Optimized for ESP LCD panels
- ✅ **PSRAM Support** - Allocates framebuffers in external PSRAM
- ✅ **ESP-BSP Integration** - Works with esp-bsp noglib components
- ✅ **2D Graphics** - Shapes, textures, text rendering, sprites

## 🎯 Supported Hardware

Currently tested on:
- **ESP32-S3-BOX-3** (320x240 ILI9341 display)

Compatible with other ESP32 boards with SPI LCD panels (may require BSP dependency updates).

## 🛠️ Requirements

- **ESP-IDF 6.0** or later
- **ESP32-S3** or similar (PSRAM recommended)
- **Board Support Package (BSP)** for your target board

## 🎮 What Can You Build?

With raylib on ESP32, you can create:
- 2D games and animations
- Interactive dashboards and HMIs
- Data visualization
- Educational demos
- Retro-style graphics applications

## 📖 Learning Resources

- [raylib official website](https://www.raylib.com/)
- [raylib cheatsheet](https://www.raylib.com/cheatsheet/cheatsheet.html)
- [raylib examples](https://www.raylib.com/examples.html)

## ⚠️ Current Limitations

- Color mapping (RGB/BGR) needs verification on some panels
- Performance: ~10-15 FPS on ESP32-S3@240MHz (software rendering)
- Audio module not yet implemented
- 3D models disabled (requires filesystem)
- Input handling (touch/buttons) not yet implemented

See [Component README](raylib/README.md) for detailed TODO list.

## 🤝 Contributing

Contributions welcome! Areas of interest:
- Additional board support
- Performance optimizations
- Input handling implementation
- Color format fixes
- Audio backend

## 📄 License

This wrapper component: **zlib/libpng** (matching raylib license)

## 🙏 Credits

- [raylib](https://www.raylib.com/) by Ramon Santamaria ([@raysan5](https://github.com/raysan5))
- Software renderer (rlsw) by Le Juez Victor ([@Bigfoot71](https://github.com/Bigfoot71))
- Software renderer merged via [raylib PR #4832](https://github.com/raysan5/raylib/pull/4832)
- ESP-IDF platform integration by this component

---

**Status**: 🚧 Work in Progress - Basic 2D graphics working, see examples for current capabilities.
