# Continuous Integration and Testing

This document describes the CI/CD pipeline for the ESP-IDF Raylib component, including automated testing via Wokwi simulation and screenshot capture.

## Overview

The project uses GitHub Actions for continuous integration with the following workflows:

- **Build and Test Raylib Examples** (`.github/workflows/build-and-test-examples.yml`)
  - Builds all examples using ESP-IDF CI action
  - Runs Wokwi simulation tests
  - Captures screenshots for supported boards
  - Uploads test artifacts (logs, screenshots)

- **Screenshot Gallery** (`.github/workflows/screenshot-gallery.yml`)
  - Runs after test workflow completes
  - Generates HTML gallery from test artifacts
  - Deploys to GitHub Pages

## Supported Boards

All examples are built and tested in CI. However, screenshot capture is only available for certain boards due to server-side limitations.

### Boards with Screenshot Support

| Board | Display | Status | Notes |
|-------|---------|--------|-------|
| ESP32-S3-BOX-3 | 320x240 ILI9341 | Fully supported | Built-in Wokwi board |
| M5Stack Core S3 | 320x240 ILI9341 | Fully supported | Built-in Wokwi board |
| M5Stack Core 2 | 320x240 ILI9342 | Fully supported | Built-in Wokwi board |

### Boards without Screenshot Support

| Board | Display | Status | Limitation |
|-------|---------|--------|-------------|
| ESP-VoCat | 360x360 ST77916 | Simulation only | Custom board SVG not accessible to server |
| ESP32-S3-Korvo-2 | 320x240 ILI9341 | Disabled | Missing GPIO expander (TCA9554) support |
| ESP32-S3-EYE | 240x240 ST7789 | Simulation only | Custom board SVG not accessible to server |
| ESP32-S3-LCD-EV | 480x480 GC9503 | Simulation only | Custom board SVG not accessible to server |
| M5Stack Atom S3 | 128x128 GC9A01 | Simulation only | Small display, custom board |
| M5Stack Atom S3R | 128x128 GC9107 | Simulation only | Small display, no BSP |
| ESP32-S3-BOX | 320x240 ST7789 | Simulation only | No BSP component available |

## Technical Limitations

### 1. Custom Board SVG Accessibility

Wokwi server loads board SVGs from `https://wokwi.github.io/wokwi-boards/`. Custom boards defined locally (e.g., in `wokwi-boards` repository) are not accessible to the server, preventing proper framebuffer rendering for screenshot capture.

**Current workaround:** Use built-in Wokwi boards for screenshot testing. Custom boards are tested for successful boot and basic functionality via serial log analysis.

**Possible solutions:**
- Host custom board SVGs publicly and submit to wokwi-boards repository
- Implement server-side support for uploading custom board SVGs
- Use only built-in boards for CI screenshot testing

### 2. GPIO Expander Support

Some boards (e.g., ESP32-S3-Korvo-2) use I/O expander chips (TCA9554) that are not simulated by Wokwi. This causes runtime crashes when the BSP attempts to initialize the expander.

**Current workaround:** Disabled from CI testing.

**Possible solutions:**
- Implement custom TCA9554 chip simulation for Wokwi
- Mock/expander detection in BSP to gracefully skip when not available
- Request Wokwi to add TCA9554 support

### 3. Screenshot API for Separate LCD Parts

The Wokwi server framebuffer API works with:
- Built-in boards with integrated displays
- Custom chips written in C using the Framebuffer API

It does NOT currently support:
- Separate LCD parts (e.g., `wokwi-ili9341`) created via board expansion

**Current workaround:** Do not expand custom boards in CI. Use built-in boards directly.

## Local Testing

### Prerequisites

```bash
# Install wokwi-cli
npm install -g wokwi-cli

# Or use custom build with custom board support
# (See wokwi-cli repository for build instructions)
```

### Running Wokwi Simulation

Each example includes a `wokwi-run.sh` script that handles board-specific configuration:

```bash
cd raylib/examples/esp32s3/espressif-esp32-s3-box-3_hello
./wokwi-run.sh
```

The script automatically:
- Detects the appropriate firmware file
- Enables screenshots for supported boards
- Loads custom board definitions when needed
- Runs with proper timeout and success criteria

### Success Criteria

Tests pass when the following string appears in serial output:
```
Starting demo loop...
```

Tests fail when:
- Backtrace/crash detected
- Timeout exceeded
- Expected string not found

## CI Artifacts

Each test run produces the following artifacts:

| Artifact | Contents | Retention |
|----------|----------|-----------|
| `flash-files-{board}-screenshot` | Screenshot PNG | 7 days |
| `flash-files-{board}-logs` | Serial log output | 7 days |
| `flash-files-{board}` | Build artifacts (ZIP) | 7 days |

## Screenshot Gallery

The screenshot gallery is automatically deployed to GitHub Pages after each CI run. It displays:

- Screenshot images for supported boards
- Test status (passed/failed) for all boards
- Timestamp of last update

Access at: `https://<username>.github.io/esp-idf-component-raylib/`

## Template System

Examples are generated from templates using `esp-generate`. The template includes:

**`wokwi-run.sh`** - Generated simulation script with board-specific options:
- Screenshot support conditional on board type
- Custom board loading via `--boards-url`
- Firmware detection for both CI and local scenarios

**Conditional syntax:**
```bash
//IF option("esp32_s3_box_3")
# Built-in board with screenshot support
WOKWI_ARGS+=(--screenshot-part "esp" --screenshot-time 10000 --screenshot-file "screen.png")
//ELIF option("esp_vocat")
# Custom board - needs external board definition
WOKWI_ARGS+=(--boards-url "https://georgik.rocks/wp-content/html5/wokwi/boards.json")
//ENDIF
```

## Future Improvements

1. **Custom Board SVG Hosting**
   - Submit custom boards to official wokwi-boards repository
   - Enable full screenshot support for all boards

2. **GPIO Expander Simulation**
   - Implement TCA9554 chip for Wokwi
   - Enable testing of Korvo-2 and similar boards

3. **Enhanced Test Coverage**
   - Add more success criteria beyond boot detection
   - Implement visual regression testing for screenshots
   - Add performance benchmarks

4. **Server-Side Support**
   - Request Wokwi to support framebuffer API for separate LCD parts
   - Implement SVG upload capability for custom boards
