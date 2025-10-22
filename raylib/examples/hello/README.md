# Hello Raylib on ESP32-S3-BOX-3 (noglib)

This example demonstrates running raylib with the software renderer on ESP-IDF using the ESP-BOX-3 (noglib) BSP.

Features:
- Software renderer (no OpenGL) via raylib rlgl OpenGL 1.1 software backend
- Double-buffered SPI transfers in chunks using esp-bsp pattern
- Simple animation and centered text

## Hardware
- Board: ESP32-S3-BOX-3
- Display: 320x240 RGB565 (ILI9341)

## Build and flash
```
idf.py @boards/esp-box-3.cfg build flash monitor
```
Notes:
- The board cfg sets target to ESP32-S3 and configures BSP/SDK options.
- PSRAM is required and used for renderer buffers.

## Expected behavior
- On startup, color test sequence: full-screen RED, GREEN, BLUE (1s each)
- Then alternating background (sky-blue / light-gray), moving red square, and centered text "Hello ESP32!"

## Known limitations (WIP)
- Color mapping still needs verification (RGB/BGR ordering)
- Text rendering may appear less crisp with software backend
- No input handling yet; the app runs continuously

## Directory structure
- `main/hello.c` Example application
- `boards/esp-box-3.cfg` Board configuration used by idf.py

## Wokwi Simulation

This example can be simulated using Wokwi (ESP32-S3-BOX-3 virtual board).

### Take Screenshot (5 seconds after startup)
```bash
wokwi-cli --diagram-file diagram.json --screenshot-part esp --screenshot-time 5000 --screenshot-file screenshot.png --timeout 6000 --timeout-exit-code 0 boards/esp-box-3
```

### Run Interactive Simulation
```bash
wokwi-cli --diagram-file diagram.json boards/esp-box-3
```

### Requirements
- [wokwi-cli](https://docs.wokwi.com/wokwi-ci/getting-started) installed
- Project built with `idf.py @boards/esp-box-3.cfg build`

**Note**: The screenshot captures the ESP32-S3-BOX-3 board (`--screenshot-part esp`) which includes the built-in LCD display.

## Troubleshooting
- If you see SPI queue errors or partial updates, ensure chunked transfers are used and PSRAM is enabled.
- If stack overflows occur, ensure the app runs in a dedicated task with a larger stack (example uses 128KB).
