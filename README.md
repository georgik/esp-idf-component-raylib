# ESP-IDF Component: Raylib

ESP-IDF wrapper component for [raylib](https://www.raylib.com/) - a simple and easy-to-use library to enjoy videogames programming.

This component enables running raylib on ESP32 microcontrollers using the **CPU-based software renderer** (no GPU required), making it possible to create graphics applications on embedded devices with LCD displays.

## Architecture: Port Layer

This implementation uses a **board-agnostic port layer** (`esp_raylib_port`) that separates display hardware management from the raylib rendering engine:

- **esp_raylib_port**: Stable API for display initialization and framebuffer operations
- **Inversion of control**: Application creates display handles and passes them to the port
- **Deterministic builds**: Board selection via Kconfig, no dynamic component injection
- **Multi-board support**: Switch boards by changing `SDKCONFIG_DEFAULTS`

## Try it Online

Test Raylib on ESP32 directly in your browser using Wokwi simulation (no hardware or build required):


[![ESP32-S3-BOX-3 Green Screen](docs/img/raylib-hello-esp-box-3-green.webp)](https://wokwi.com/experimental/viewer?diagram=https%3A%2F%2Fraw.githubusercontent.com%2Fgeorgik%2Fesp-idf-component-raylib%2Fmain%2Fraylib%2Fexamples%2Fhello%2Fwokwi%2Fesp-box-3%2Fdiagram.json&firmware=https%3A%2F%2Fgithub.com%2Fgeorgik%2Fesp-idf-component-raylib%2Freleases%2Fdownload%2Fv5.6.0%2Fraylib-hello-v5.6.0-esp-box-3.bin)

[![ESP32-S3-BOX-3 Hello World](docs/img/raylib-hello-esp-box-3-hello.webp)](https://wokwi.com/experimental/viewer?diagram=https%3A%2F%2Fraw.githubusercontent.com%2Fgeorgik%2Fesp-idf-component-raylib%2Fmain%2Fraylib%2Fexamples%2Fhello%2Fwokwi%2Fesp-box-3%2Fdiagram.json&firmware=https%3A%2F%2Fgithub.com%2Fgeorgik%2Fesp-idf-component-raylib%2Freleases%2Fdownload%2Fv5.6.0%2Fraylib-hello-v5.6.0-esp-box-3.bin)

**[Run ESP32-S3-BOX-3 (320x240)](https://wokwi.com/experimental/viewer?diagram=https%3A%2F%2Fraw.githubusercontent.com%2Fgeorgik%2Fesp-idf-component-raylib%2Fmain%2Fraylib%2Fexamples%2Fhello%2Fwokwi%2Fesp-box-3%2Fdiagram.json&firmware=https%3A%2F%2Fgithub.com%2Fgeorgik%2Fesp-idf-component-raylib%2Freleases%2Fdownload%2Fv5.6.0%2Fraylib-hello-v5.6.0-esp-box-3.bin)**


## Flash Pre-built Binaries

No build required! Flash pre-built binaries directly from your browser using ESP Launchpad:

[![Try it with ESP Launchpad](https://espressif.github.io/esp-launchpad/assets/try_with_launchpad.png)](https://georgik.github.io/esp-idf-component-raylib/?flashConfigURL=https://georgik.github.io/esp-idf-component-raylib/config/config.toml)

**Supported boards:**
- ESP32-S3-BOX-3

**Requirements:**
- Chrome or Edge browser (WebSerial support required)
- USB cable connected to your board

## For Developers of ESP-IDF Component

### Quick Start for Development

Clone the repository and build in-place (no publishing required):

```bash
# Clone the repository
git clone --recurse-submodules https://github.com/georgik/esp-idf-component-raylib.git
cd esp-idf-component-raylib

# Navigate to the example
cd raylib/examples/hello

# Set your target chip
idf.py set-target esp32s3

# Build the example (uses local components automatically)
idf.py build

# Flash to your board
idf.py -p /dev/ttyUSB0 flash monitor
```

**How it works:** The example's `idf_component.yml` uses `override_path` to reference the local `raylib` and `esp_raylib_port` components. Changes you make to the component source code are immediately reflected when you rebuild - no need to publish or create separate projects.

### Development Workflow

1. **Make changes** to component source code in `raylib/` or `esp_raylib_port/`
2. **Rebuild** the example: `cd raylib/examples/hello && idf.py build`
3. **Test** on hardware or in Wokwi simulator
4. **Iterate** - changes take effect immediately

### Board-Specific Testing

Test with different board configurations:

```bash
cd raylib/examples/hello

# ESP-BOX-3
cp sdkconfig.defaults.esp-box-3 sdkconfig.defaults
idf.py set-target esp32s3
idf.py build

# ESP32-P4
cp sdkconfig.defaults.esp32_p4_function_ev_board sdkconfig.defaults
idf.py set-target esp32p4
idf.py build
```

### Clean Build

To ensure a clean build after making changes:

```bash
cd raylib/examples/hello
idf.py fullclean
idf.py build
```

### Verifying Component Dependencies

Check that components are resolved from local paths:

```bash
cd raylib/examples/hello
idf.py reconfigure 2>&1 | grep -i "component\|dependency"
```

Expected output should show paths like:
```
/Users/georgik/projects/esp-idf-component-raylib/esp_raylib_port
/Users/georgik/projects/esp-idf-component-raylib/raylib
```

### CI/CD Testing

For automated testing across multiple configurations, use `idf-build-apps`:

```bash
pip install idf-build-apps
idf-build-apps build raylib/examples
```

## Build from Source (For Users)

Users installing this component from the component registry can build examples with:

```bash
cd raylib/examples/hello

# Method 1: Explicit SDKCONFIG_DEFAULTS
idf.py set-target esp32s3
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.defaults.esp-box-3" reconfigure build flash monitor

# Method 2: Using ESPBrew (recommended for multi-board development)
espbrew --cli-only .
```

## Repository Structure

```
esp-idf-component-raylib/
├── README.md                  # This file
├── esp_raylib_port/           # Port layer component (georgik/esp_raylib_port)
│   ├── idf_component.yml      # Published separately
│   └── src/                   # Board-agnostic display API
└── raylib/                    # Main component (georgik/raylib)
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

## Documentation

- **[Component Documentation](raylib/README.md)** - Architecture, configuration, and API
- **[Hello Example](raylib/examples/hello/README.md)** - Complete working example with build instructions

## Features

- **Software Rendering** - No OpenGL/GPU required, runs entirely on CPU
- **RGB565 Framebuffer** - Optimized for ESP LCD panels
- **PSRAM Support** - Allocates framebuffers in external PSRAM
- **ESP-BSP Integration** - Works with esp-bsp noglib components
- **2D Graphics** - Shapes, textures, text rendering, sprites

## Supported Hardware

**Fully tested and working:**
- **ESP32-S3-BOX-3** (320x240 ILI9341, SPI)
- **ESP32-P4 Function EV Board** (1024x600, MIPI-DSI)
- **M5Stack Core S3** (320x240, SPI)

**Easy to add:** Any board with ESP-BSP noglib support or custom esp_lcd panel.

## Requirements

- **ESP-IDF 5.5+** (tested with 6.0)
- **ESP32-S3** or **ESP32-P4** (PSRAM recommended)
- **Board Support Package (BSP)** noglib version for your board

## What Can You Build?

With raylib on ESP32, you can create:
- 2D games and animations
- Interactive dashboards and HMIs
- Data visualization
- Educational demos
- Retro-style graphics applications

## Learning Resources

- [raylib official website](https://www.raylib.com/)
- [raylib cheatsheet](https://www.raylib.com/cheatsheet/cheatsheet.html)
- [raylib examples](https://www.raylib.com/examples.html)

## Current Limitations

- Audio module not yet implemented
- 3D models disabled (requires filesystem)
- Touch input framework ready but not yet connected
- Large displays (1024x600) render slowly at full resolution

See [Component README](raylib/README.md) for detailed information.

## Contributing

Contributions welcome! Areas of interest:
- Additional board support
- Performance optimizations
- Input handling implementation
- Color format fixes
- Audio backend

## License

This wrapper component: **zlib/libpng** (matching raylib license)

## Credits

- [raylib](https://www.raylib.com/) by Ramon Santamaria ([@raysan5](https://github.com/raysan5))
- Software renderer (rlsw) by Le Juez Victor ([@Bigfoot71](https://github.com/Bigfoot71))
- Software renderer merged via [raylib PR #4832](https://github.com/raysan5/raylib/pull/4832)
- ESP-IDF platform integration by this component

## Documentation

- **[Component Documentation](raylib/README.md)** - Architecture, configuration, and API
- **[Hello Example](raylib/examples/hello/README.md)** - Complete working example
- **[Implementation Guide](PLAN_A_IMPLEMENTATION.md)** - Detailed user guide and API reference
