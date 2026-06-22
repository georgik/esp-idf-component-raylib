# Raylib Hello Example Template

A template for generating raylib hello examples for various ESP32 development boards.

## Board Configuration Source

**Source of truth:** ESP-IDF BSP JSON files at `/path/to/esp-bsp/bsp/<board_name>/<board_name>.json`

Each board's display configuration (resolution, panel type, color order, interface) comes from these JSON files. When adding new boards, reference the corresponding JSON for accurate configuration.

## Supported Boards

| Board | MCU | Resolution | Panel | Interface | Notes |
|-------|-----|------------|-------|-----------|-------|
| ESP32-S3-BOX-3 | ESP32-S3 | 320x240 | ILI9341 | SPI | BSP: esp-box-3_noglib |
| ESP32-S3-LCD-EV-Board | ESP32-S3 | 480x480 | GC9503 | SPI | BSP: esp32_s3_lcd_ev_board |
| ESP32-S3-BOX | ESP32-S3 | 320x240 | ST7789 | SPI | **No BSP** - uses direct esp_lcd |
| ESP32-S3-Korvo-2 | ESP32-S3 | 320x240 | ILI9341 | SPI | BSP: esp32_s3_korvo_2 |
| ESP-VoCat | ESP32-S3 | 360x360 | ST77916 | QSPI | BSP: esp_vocat |
| ESP32-P4-Function-EV-Board | ESP32-P4 | 1024x600 | MIPI | MIPI-DSI | **P4 arch**, dual-resolution |
| ESP32-S3-EYE | ESP32-S3 | 240x240 | ST7789 | SPI | BSP: esp32_s3_eye, built-in driver |
| M5Stack-Core2 | ESP32-S3 | 320x240 | ILI9342 | SPI | BSP: m5stack_core_2 |
| M5Stack-CoreS3 | ESP32-S3 | 320x240 | ILI9341 | SPI | BSP: m5stack_core_s3, IO expander |
| M5Stack-Tab5 | ESP32-P4 | 720x1280 | ST7123 | MIPI-DSI | **P4 arch**, IO expander |
| ESP32-P4-EYE | ESP32-P4 | 240x240 | ST7789 | SPI | **P4 arch**, BSP: esp32_p4_eye |

**Architecture Notes:**
- **ESP32-P4 boards** (P4) use RISC-V architecture - not yet tested with raylib
- **IO expander boards** (CoreS3, Tab5) use AW9523/PI4IOE5V6408 for GPIO control
- **No BSP boards** (ESP32-S3-BOX) use direct esp_lcd initialization

## Usage

Generate an example for a specific board:

```bash
esp-generate --bsp /path/to/raylib-hello-c-template -o <board_id> my_board_hello
```

## Building

After generation:

```bash
cd my_board_hello/main
idf.py build flash monitor
```

## Adding New Boards

1. Find board JSON in `esp-bsp/bsp/<board>/<board>.json`
2. Extract display config: `BSP_LCD_H_RES`, `BSP_LCD_V_RES`, `BSP_LCD_COLOR_SPACE`, `BSP_LCD_BIGENDIAN`
3. Update `template.yaml` with board option
4. Update `main/idf_component.yml` with BSP dependency (if exists)
5. Update `main/main.c` with conditional includes and dimension callback
