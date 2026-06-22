# Raylib Hello Example Template

This directory contains the template for generating board-specific raylib hello examples using esp-generate.

## Template Structure

```
raylib-hello-c/
├── template.yaml              # Board options and metadata
├── main/
│   ├── idf_component.yml      # Component dependencies (conditional)
│   ├── main.c                 # Main code with conditional compilation
│   └── README.md
└── CMakeLists.txt
```

## How It Works

The template uses esp-generate's conditional compilation syntax to generate board-specific examples:

1. **template.yaml**: Defines available board options
2. **Conditional compilation**: `//IF`, `//ELIF`, `//ELSE`, `//ENDIF` directives
3. **Generation**: esp-generate processes the template and creates board-specific code

## Conditional Compilation Syntax

```c
//IF option("board_name")
    // Code for this board only
//ELIF option("other_board")
    // Code for other board
//ELSE
    // Default code for all other boards
//ENDIF
```

## Current Board Configuration

**BSP Boards (use esp-bsp components):**
- esp32_s3_box_3: ESP32-S3-BOX-3 (320x240 ILI9341)
- esp32_s3_lcd_ev_board: ESP32-S3-LCD-EV (480x480 GC9503)
- esp32s3_korvo_2: ESP32-S3-Korvo-2 (320x240 ILI9341)
- esp_vocat: ESP-VoCat (360x360 ST77916)
- esp32_s3_eye: ESP32-S3-EYE (240x240 ST7789)
- m5stack_core_s3: M5Stack-CoreS3 (320x240 ILI9341)
- m5stack_atom_s3: M5Stack-AtomS3 (128x128 GC9A01)
- m5stack_core_2: M5Stack-Core2 (320x240 ILI9342)

**Direct Init Boards (no BSP):**
- esp32_s3_box: ESP32-S3-BOX (320x240 ST7789)
- m5stack_atom_s3r: M5Stack-AtomS3R (128x128 GC9107)

## Adding a New Board

To add a new board to the template:

### 1. Add Board Option (template.yaml)

```yaml
- !Option
  name: my_new_board
  display_name: My New Board (320x240 ST7789)
  chips: [esp32s3]
```

### 2. Add Component Dependency (main/idf_component.yml)

For BSP boards:
```yaml
#IF option("my_new_board")
espressif/my_board_bsp:
  version: "*"
#ENDIF
```

For direct init boards (no BSP):
```yaml
#IF option("my_new_board")
# No BSP component - uses direct esp_lcd
#ENDIF
```

### 3. Add Board-Specific Code (main/main.c)

**TAG definition:**
```c
//ELIF option("my_new_board")
static const char *TAG = "MY_NEW_BOARD";
```

**Display dimensions:**
```c
//ELIF option("my_new_board")
if (w) *w = 320;
if (h) *h = 240;
```

**InitWindow call:**
```c
//ELIF option("my_new_board")
InitWindow(w, h, "My New Board Raylib");
```

**Message string:**
```c
//ELIF option("my_new_board")
const char *msg = "My New Board";
```

**Display init (BSP or direct):**

For BSP boards, add to the BSP init chain:
```c
//ELIF option("my_new_board")
bsp_display_config_t cfg = {
    .max_transfer_sz = 320 * 48 * sizeof(uint16_t),
};
esp_err_t ret = bsp_display_new(&cfg, &g_panel, &g_io);
// ... error handling
esp_lcd_panel_disp_on_off(g_panel, true);
bsp_display_backlight_on();
```

For direct init boards, add a new block with pin definitions:
```c
//ELIF option("my_new_board")
#define MY_BOARD_LCD_MOSI  GPIO_NUM_21
#define MY_BOARD_LCD_SCLK  GPIO_NUM_15
// ... pin definitions
spi_bus_config_t bus_cfg = { ... };
// ... SPI init
esp_lcd_new_panel_st7789(...);
// ... panel init
```

### 4. Update Regeneration Script

Add the board to the BOARDS array in `raylib/scripts/regenerate-all.sh`:
```bash
BOARDS=(
    "my_new_board:My New Board"
    # ... existing boards
)
```

### 5. Regenerate Examples

```bash
cd raylib
./scripts/regenerate-all.sh
```

### 6. Test New Example

```bash
cd raylib/examples/my_new_board_hello
idf.py set-target esp32s3
idf.py build flash monitor
```

## Display Driver References

- ST7789: Built into ESP-IDF 6
- ILI9341: espressif/esp_lcd_ili9341 component
- GC9A01: espressif/esp_lcd_gc9a01 component
- GC9107: espressif/esp_lcd_gc9107 component
- GC9503: espressif/esp_lcd_gc9503 component
- ST77916: espressif/esp_lcd_st77916 component

## BSP Components

BSP components use the "_noglib" suffix (no LVGL dependency):
- espressif/esp-box-3_noglib
- espressif/m5_atom_s3_noglib
- espressif/m5stack_core_s3
- espressif/m5stack_core_2
- etc.

## Board Configuration Source

**Source of truth:** ESP-IDF BSP JSON files at `/path/to/esp-bsp/bsp/<board_name>/<board_name>.json`

Each board's display configuration (resolution, panel type, color order, interface) comes from these JSON files. When adding new boards, reference the corresponding JSON for accurate configuration.
