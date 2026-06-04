# Raylib Hello Example Template

A template for generating raylib hello examples for various ESP32 development boards.

## Supported Boards

- ESP32-S3-BOX-3 (320x240 ILI9341)
- ESP32-S3-LCD-EV-Board (480x480 GC9503)
- ESP-BOX (320x240 ILI9342)
- ESP32-S3-Korvo-1 (320x240 ILI9341)
- ESP32-S3-Korvo-2 (320x240 ILI9341)

## Usage

Generate an example for a specific board:

```bash
esp-generate --bsp /path/to/raylib-hello-c-template -o esp32s3_box_3 my_board_hello
```

## Building

After generation:

```bash
cd my_board_hello/main
idf.py build flash monitor
```
