# Raylib Hello World Example

Simple raylib example displaying colored text on the screen.

## Building

Build with ESP-IDF:
```bash
idf.py build
```

Flash to device:
```bash
idf.py flash monitor
```

## Hardware

This example is configured for specific development boards with display support.
See the board-specific documentation for pin configurations and display details.

### Display (M5Stack AtomS3R)

The AtomS3R carries a small **GC9107** LCD (128x128, RGB565 over SPI3_HOST).
ESP-IDF ships no dedicated GC9107 driver, so this example drives it with the
register-compatible **`esp_lcd_gc9a01`** component.

The display init carries the validated fixes from the working reference firmware
(`m5stack-atom-s3r-joystick-usb`):

- GC9107 via the GC9A01 driver (the ST7789 driver sends Sitronix commands the
  GC9107 ignores, producing a broken/incorrectly coloured image).
- 500 ms power-on settle **before** reset and 200 ms **after** reset, so the
  MADTL/init commands are not dropped (avoids a split / low-res display).
- `esp_lcd_panel_mirror(panel, true, false)` (MX column flip only) for the
  board's hardware-mirrored panel.
- `esp_lcd_panel_invert_color(panel, true)` (INVON) — the panel boots inverted.
- A single full-frame PSRAM flush (raylib's vertical flip is undone per row and
  pixels are byte-swapped for big-endian SPI). Dimensions are reported as
  130x129 so the flush can draw slightly past the physical 128x128 panel.

The LCD backlight is powered by an **LP5562** I2C constant-current LED driver
(addr 0x30, SDA=GPIO45, SCL=GPIO0). `backlight_init()` enables it — without it
the panel stays dark even though the LCD controller is initialised.

> **HW revision note:** this is configured for HW rev **2025.09.19**. Newer HW
> revisions require the ST7789 driver instead (not implemented in this example).

## Wokwi Simulation

This example also supports Wokwi simulation for testing without hardware.

Install wokwi-cli: https://docs.wokwi.com/wokwi-ci/cli-installation

Build and run:
```bash
idf.py build
wokwi-cli
```
