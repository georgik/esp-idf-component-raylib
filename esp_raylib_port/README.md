# ESP Raylib Port

Board-agnostic port layer for [Raylib](https://www.raylib.com/) on ESP-IDF, providing a stable abstraction between the Raylib graphics library and ESP32 display hardware.

## Overview

`esp_raylib_port` is a standalone ESP-IDF component that bridges Raylib applications with ESP-IDF display drivers (`esp_lcd`). It handles:

- **RGB565 framebuffer management**: Optimized for ESP LCD panels
- **Display synchronization**: Semaphore-based coordination for SPI and DSI panels
- **Byte-order conversion**: Automatic swapping for SPI panels vs DSI panels
- **Chunked transfers**: Efficient SPI data transmission
- **Thread-safe operations**: Mutex-protected API
- **Performance monitoring**: Optional frame rate and bandwidth logging

## Architecture

```
Application (raylib)
    ↓
esp_raylib_port (this component)
    ↓
esp_lcd (ESP-IDF display driver)
    ↓
Hardware (LCD panel)
```

**Key Design Principles:**
- **Inversion of Control**: Application creates panel handles and passes them to the port
- **Board Agnostic**: No hard-coded board-specific code
- **Stable API**: Decouples Raylib from ESP-IDF display changes
- **Zero Configuration**: Sensible defaults with override options

## Installation

### Using ESP Component Registry

```yaml
# idf_component.yml
dependencies:
  georgik/esp_raylib_port: "^1.0.0"
```

### Local Development

```yaml
# idf_component.yml
dependencies:
  georgik/esp_raylib_port:
    version: "*"
    override_path: "../esp_raylib_port"  # Point to local copy
```

## Quick Start

### 1. Initialize the Port Layer

```c
#include "esp_raylib_port.h"

// Configure port
ray_port_cfg_t port_cfg = {
    .buff_psram = true,          // Allocate framebuffer in PSRAM
    .swap_rgb_bytes = true,      // true for SPI panels, false for DSI
    .hres = 320,
    .vres = 240,
    .perf_stats = true,          // Enable FPS logging
};

ESP_ERROR_CHECK(ray_port_init(&port_cfg));
```

### 2. Register Display Panel

```c
// Create panel using ESP-BSP or esp_lcd
esp_lcd_panel_handle_t panel_handle = /* ... */;
esp_lcd_panel_io_handle_t io_handle = /* ... */;

// Register with port
ray_port_display_cfg_t disp_cfg = {
    .panel = panel_handle,
    .io = io_handle,
    .hres = 320,
    .vres = 240,
    .dma_capable = true,
};

ESP_ERROR_CHECK(ray_port_add_display(&disp_cfg));
```

### 3. Use with Raylib

The port layer automatically handles all framebuffer operations when used with the `georgik/raylib` component:

```c
#include "raylib.h"

InitWindow(320, 240, "Hello");

while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(SKYBLUE);
    DrawText("Hello ESP32!", 10, 10, 20, BLACK);
    EndDrawing();
}

CloseWindow();
```

## API Reference

### Initialization

#### `ray_port_init()`
```c
esp_err_t ray_port_init(const ray_port_cfg_t* cfg);
```

Initialize the port layer. Must be called before any other port functions.

**Parameters:**
- `cfg` - Port configuration

**Configuration Options:**
```c
typedef struct {
    bool buff_psram;          // Allocate framebuffer in PSRAM (recommended)
    bool double_buffer;       // Enable double buffering (future)
    size_t buffer_pixels;     // Buffer size in pixels (0 = full screen)
    size_t chunk_bytes;       // Max chunk for SPI (0 = auto: 48 lines)
    bool swap_rgb_bytes;      // Swap byte order (true for SPI, false for DSI)
    uint16_t hres;            // Horizontal resolution
    uint16_t vres;            // Vertical resolution
    int rotation;             // Display rotation (0, 90, 180, 270)
    bool perf_stats;          // Enable performance logging
} ray_port_cfg_t;
```

### Display Management

#### `ray_port_add_display()`
```c
esp_err_t ray_port_add_display(const ray_port_display_cfg_t* disp);
```

Register an initialized display panel with the port.

**Parameters:**
```c
typedef struct {
    esp_lcd_panel_handle_t panel;      // ESP-LCD panel handle (required)
    esp_lcd_panel_io_handle_t io;      // ESP-LCD IO handle (optional)
    uint16_t hres;                      // Horizontal resolution
    uint16_t vres;                      // Vertical resolution
    bool monochrome;                    // Monochrome display (not yet supported)
    bool dma_capable;                   // Display supports DMA
} ray_port_display_cfg_t;
```

#### `ray_port_remove_display()`
```c
esp_err_t ray_port_remove_display(void);
```

Unregister the current display panel.

#### `ray_port_get_dimensions()`
```c
esp_err_t ray_port_get_dimensions(uint16_t* w, uint16_t* h);
```

Query display dimensions.

### Framebuffer Operations

#### `ray_port_flush_rgb565()`
```c
esp_err_t ray_port_flush_rgb565(const uint16_t* buf, uint16_t x, uint16_t y, 
                                 uint16_t w, uint16_t h);
```

Flush RGB565 framebuffer region to display. Automatically handles:
- Byte swapping (if configured)
- Chunked transfers (for SPI)
- Synchronization via semaphores

### Thread Safety

#### `ray_port_lock()`
```c
void ray_port_lock(uint32_t timeout_ms);
```

Lock port for thread-safe operations.

#### `ray_port_unlock()`
```c
void ray_port_unlock(void);
```

Unlock port.

### Cleanup

#### `ray_port_deinit()`
```c
esp_err_t ray_port_deinit(void);
```

Deinitialize the port layer and free resources.

## Panel Type Configuration

### SPI Panels (ESP32-S3-BOX-3, M5Stack Core S3)
```c
ray_port_cfg_t cfg = {
    .swap_rgb_bytes = true,   // ✅ Byte swapping required
    .chunk_bytes = 0,         // Auto-chunking (48 lines)
    // ...
};
```

### DSI Panels (ESP32-P4 Function EV Board)
```c
ray_port_cfg_t cfg = {
    .swap_rgb_bytes = false,  // ❌ No byte swapping needed
    .chunk_bytes = 0,         // No chunking (direct transfer)
    // ...
};
```

## Performance Tuning

### Framebuffer Allocation
- **Always use PSRAM**: `buff_psram = true`
- Stack allocation will cause overflow on ESP32

### Chunk Size (SPI only)
```c
// Default (recommended): 48 lines per chunk
.chunk_bytes = 0,  

// Custom: e.g., 320×24×2 = 15,360 bytes per chunk
.chunk_bytes = 320 * 24 * sizeof(uint16_t),
```

### Performance Statistics
```c
.perf_stats = true,  // Logs every 60 frames
```

Output:
```
I (5000) ray_port: Flush #60: 320x240 in 45.2 ms (22.1 FPS, 3200.5 KB/s)
```

## Example: Complete Integration

See the [Raylib hello example](../raylib/examples/hello/) for a complete working example showing:
- Board-specific initialization using ESP-BSP
- Port layer configuration
- Raylib integration
- Multi-board support (ESP-BOX-3, M5Stack Core S3, ESP32-P4)

## Supported Hardware

The port layer is hardware-agnostic and works with any ESP-LCD panel:

**Tested Boards:**
- ESP32-S3-BOX-3 (320×240, ILI9341, SPI) ✅
- M5Stack Core S3 (320×240, ILI9342C, SPI) ✅
- ESP32-P4 Function EV Board (1024×600, EK79007, MIPI-DSI) ✅

**Easy to Add:** Any board with `esp_lcd` panel support.

## Requirements

- **ESP-IDF**: 5.5.0 or later
- **PSRAM**: Highly recommended for framebuffer allocation
- **Board Support**: ESP-LCD panel handle from ESP-BSP or custom initialization

## Known Limitations

- Touch input framework present but not yet connected to Raylib
- Monochrome displays not yet supported
- Double buffering not yet implemented
- Performance: ~15-20 FPS on ESP32-S3@240MHz (320×240)

## Integration with Raylib Component

This component is designed to work with the `georgik/raylib` ESP-IDF component:

```yaml
# idf_component.yml
dependencies:
  georgik/raylib: "^5.6.0"           # Automatically pulls esp_raylib_port
  # OR explicitly:
  georgik/esp_raylib_port: "^1.0.0"  # Use port independently
```

The Raylib component automatically uses `esp_raylib_port` for display operations.

## Contributing

Contributions welcome! Areas of interest:
- Touch input integration
- Double buffering implementation
- Additional panel type support
- Performance optimizations
- Color format extensions

## License

**zlib/libpng** (matching Raylib license)

## Credits

- Part of the [esp-idf-component-raylib](https://github.com/georgik/esp-idf-component-raylib) project
- Inspired by [esp_lvgl_port](https://github.com/espressif/esp-bsp/tree/master/components/esp_lvgl_port) from ESP-BSP

## See Also

- [Raylib Component](../raylib/README.md) - Main Raylib ESP-IDF wrapper
- [Hello Example](../raylib/examples/hello/README.md) - Complete working example
- [Raylib Official](https://www.raylib.com/) - Raylib graphics library
