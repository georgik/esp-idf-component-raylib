/**
 * @file main.c
 * @brief Raylib example
 *
 * Architecture:
 * - Display driven directly via esp_lcd (AtomS3R has no BSP component here)
 * - GC9107 panel (register-compatible with GC9A01) via esp_lcd_gc9a01
 * - LP5562 I2C LED driver powers the backlight so the panel actually lights up
 * - rcore callback interface connects raylib to display
 *
 * Board configurations sourced from esp-bsp JSON files:
 * /path/to/esp-bsp/bsp/<board_name>/<board_name>.json
 *
 * Display init / flush carry the validated fixes from the working reference
 * firmware (m5stack-atom-s3r-joystick-usb): GC9107 via the GC9A01 driver,
 * power-on settle timing around reset, MX-only mirror, colour inversion
 * (INVON), and a single full-frame PSRAM flush. See wiki/display.md.
 */

#include "esp_log.h"
#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_gc9a01.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "backlight.h"
#include "raylib.h"

static const char *TAG = "M5STACK_ATOMS3R";

// Display handles from BSP
static esp_lcd_panel_handle_t g_panel = NULL;
static esp_lcd_panel_io_handle_t g_io = NULL;

/**
 * @brief Display flush callback for rcore
 *
 * raylib's SwapScreenBuffer vertically flips the framebuffer, so buf[i]
 * holds the natural-image row (h-1-i). We rebuild a natural-order DMA
 * buffer (row 0 = top of the scene) so the GC9107 writes it in order.
 * RGB565 is byte-swapped per pixel (__builtin_bswap16): little-endian
 * ESP32 -> big-endian SPI, matching mipidsi.
 *
 * The panel is physically 128x128 but we report 130x129 to raylib (see
 * display_get_dimensions()) and draw the window (x,y)-(x+w,y+h) with NO COG
 * offset. The +2px width covers the panel's spare right-edge pixels and the
 * +1px height removes the "unused pixels at the bottom" / alignment artifact.
 * See wiki/display.md §6.4 / §6.5.
 */
static void display_flush(const uint16_t *buf, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    if (!g_panel || !buf) {
        return;
    }

    // Draw the whole framebuffer naturally (top-first), in one call, exactly as
    // the validated reference firmware does. Splitting it into per-chunk
    // draw_bitmap calls made the GC9107 re-set RASET and wrap/repeat in the
    // lower third, because the last chunk's window end (y+129) exceeds the
    // 128-row panel. A single full-frame draw avoids that.
    //
    // release/v6.1 reserves a 32 KB internal DMA pool; the full 130x129
    // framebuffer (~33.5 KB) cannot fit in contiguous DMA-capable internal
    // SRAM, so heap_caps_malloc(MALLOC_CAP_DMA) fails with ESP_ERR_NO_MEM and
    // the SPI priv-TX-buffer path aborts. We therefore allocate the flush
    // framebuffer in PSRAM. With psram_dma_direct=1 on the panel IO (see
    // init_display), the SPI DMA path reads PSRAM directly and needs no
    // internal scratch buffer at all, so the DMA error disappears. Fall back
    // to plain SRAM only if PSRAM is unavailable.
    const size_t size = (size_t)h * w * sizeof(uint16_t);
    uint16_t *fb = heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
    if (!fb) {
        fb = malloc(size);   // plain SRAM fallback
    }
    if (!fb) {
        ESP_LOGE(TAG, "Failed to allocate flush buffer");
        return;
    }

    // Rebuild a natural-order framebuffer: undo raylib's vertical flip so that
    // fb[r] is the r-th row of the drawn scene (r = 0 is the top).
    for (uint16_t r = 0; r < h; r++) {
        uint16_t src_row = (uint16_t)(h - 1 - r);
        const uint16_t *sp = buf + ((uint32_t)src_row * w);
        uint16_t *dp = fb + ((uint32_t)r * w);
        for (uint16_t c = 0; c < w; c++) {
            dp[c] = __builtin_bswap16(sp[c]);   // little-endian ESP32 -> big-endian SPI LCD
        }
    }

    // Draw the full framebuffer directly, with no COG offset (wiki/display.md §6.4).
    esp_err_t ret = esp_lcd_panel_draw_bitmap(g_panel, x, y, x + w, y + h, fb);
    heap_caps_free(fb);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to draw bitmap: %s", esp_err_to_name(ret));
    }
}

/**
 * @brief Get display dimensions callback.
 *
 * The panel is physically 128x128; we report 130x129 so the flush can draw a
 * framebuffer 2px wider / 1px taller than the panel (wiki/display.md §6.4).
 */
static void display_get_dimensions(uint16_t *w, uint16_t *h)
{
    if (w) *w = 130;
    if (h) *h = 129;
}

extern void raylib_esp_set_display_callbacks(
    void (*flush_fn)(const uint16_t *buf, uint16_t x, uint16_t y, uint16_t w, uint16_t h),
    void (*get_dim_fn)(uint16_t *w, uint16_t *h)
);

#define RAYLIB_TASK_STACK_SIZE (128 * 1024)

/**
 * @brief Initialize the GC9107 display (SPI3_HOST) + LP5562 backlight.
 *
 * Timing / settings mirror the working Rust mipidsi firmware (see
 * wiki/display.md §6.2 / §6.4.1): 500ms power-on settle BEFORE reset, 200ms
 * AFTER reset, MADTL 0x48 (mirror(true,false)), INVON colour.
 */
static esp_err_t init_display(void)
{
    ESP_LOGI(TAG, "Initializing display...");

    // AtomS3R display is a GC9107 (register-compatible with GC9A01). The ST7789
    // driver sends Sitronix init commands the GC9107 ignores, so we use
    // esp_lcd_gc9a01. Pinout from M5Stack AtomS3R hardware docs.
    //
    // NOTE: this is for HW rev 2025.09.19. Newer HW revisions require the ST7789
    // driver instead (not implemented in this example yet).
    #define M5STACK_ATOM_S3R_LCD_MOSI      GPIO_NUM_21
    #define M5STACK_ATOM_S3R_LCD_SCLK      GPIO_NUM_15
    #define M5STACK_ATOM_S3R_LCD_CS        GPIO_NUM_14
    #define M5STACK_ATOM_S3R_LCD_DC        GPIO_NUM_42
    #define M5STACK_ATOM_S3R_LCD_RST       GPIO_NUM_48

    // SPI bus configuration (ESP-IDF 6 API)
    spi_bus_config_t bus_cfg = {
        .sclk_io_num = M5STACK_ATOM_S3R_LCD_SCLK,
        .mosi_io_num = M5STACK_ATOM_S3R_LCD_MOSI,
        .miso_io_num = GPIO_NUM_NC,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = 128 * 128 * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &bus_cfg, SPI_DMA_CH_AUTO));

    // LCD IO SPI configuration (ESP-IDF 6 API)
    esp_lcd_panel_io_spi_config_t io_cfg = {
        .cs_gpio_num = M5STACK_ATOM_S3R_LCD_CS,
        .dc_gpio_num = M5STACK_ATOM_S3R_LCD_DC,
        .spi_mode = 0,
        .pclk_hz = 40 * 1000 * 1000,
        .trans_queue_depth = 10,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .flags = {
            .dc_low_on_data = 0,
            // release/v6.1: read the color (draw_bitmap) buffer directly from
            // PSRAM instead of staging it into a small internal DMA buffer.
            // This is what makes the PSRAM-allocated flush framebuffer usable
            // and avoids the "Failed to allocate priv TX buffer" failure.
            .psram_dma_direct = 1,
        },
    };

    // Create panel IO (ESP-IDF 6 API - uses SPI3_HOST directly)
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI3_HOST, &io_cfg, &g_io));

    // GC9107 panel configuration
    esp_lcd_panel_dev_config_t panel_cfg = {
        .reset_gpio_num = M5STACK_ATOM_S3R_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel = 16,
    };

    // Initialize GC9107 panel via the GC9A01 driver
    ESP_ERROR_CHECK(esp_lcd_new_panel_gc9a01(g_io, &panel_cfg, &g_panel));

    // Fix: 500ms power-on settle BEFORE reset/init (mipidsi behaviour).
    // Without this, MADTL/init commands get lost -> split / low-res display.
    vTaskDelay(pdMS_TO_TICKS(500));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(g_panel));

    // Fix: 200ms settle AFTER reset, BEFORE init command seq.
    // Sending init commands too soon after reset drops them -> bad resolution.
    vTaskDelay(pdMS_TO_TICKS(200));

    ESP_ERROR_CHECK(esp_lcd_panel_init(g_panel));

    // MADTL 0x48: this board's GC9107 is hardware horizontally mirrored, so the
    // MX column-flip bit is required to produce a non-mirrored image.
    // (The MY bit tried earlier only flips rows and did not fix the mirror.)
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(g_panel, true, false));

    // INVON colour: the panel boots inverted and the GC9107 vendor table
    // reinforces it, so INVOFF cannot clear it — use INVON (wiki/display.md §6.6).
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(g_panel, true));

    // Turn on display
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(g_panel, true));

    // Small delay for display to stabilize
    vTaskDelay(pdMS_TO_TICKS(100));

    // AtomS3R backlight: the white-backlight LEDs are powered by an LP5562
    // constant-current LED driver on I2C (addr 0x30, SDA=GPIO45, SCL=GPIO0).
    // The working reference firmware enables it explicitly; without this the
    // panel is dark even though the LCD controller above is initialised. This
    // is the primary cause of "screen not initialized".
    ESP_ERROR_CHECK(backlight_init());

    // Register display callbacks with rcore
    raylib_esp_set_display_callbacks(display_flush, display_get_dimensions);

    uint16_t w, h;
    display_get_dimensions(&w, &h);
    ESP_LOGI(TAG, "Display initialized: %dx%d", w, h);
    return ESP_OK;
}

void raylib_task(void *pvParameter)
{
    ESP_LOGI(TAG, "Initializing Raylib...");

    uint16_t w, h;
    display_get_dimensions(&w, &h);
    InitWindow(w, h, "M5Stack-AtomS3R Raylib");

    ESP_LOGI(TAG, "Starting demo loop...");

    // Color test. With the GC9107 correctly initialised (BGR + INVON + MX
    // mirror) these should render as the colours the names imply.
    BeginDrawing();
    ClearBackground(RED);
    DrawText("RED", 10, 10, 20, WHITE);
    EndDrawing();
    vTaskDelay(pdMS_TO_TICKS(1000));

    BeginDrawing();
    ClearBackground(GREEN);
    DrawText("GREEN", 10, 10, 20, WHITE);
    EndDrawing();
    vTaskDelay(pdMS_TO_TICKS(1000));

    BeginDrawing();
    ClearBackground(BLUE);
    DrawText("BLUE", 10, 10, 20, WHITE);
    EndDrawing();
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Main loop
    int frame = 0;
    while (!WindowShouldClose()) {
        BeginDrawing();

        if ((frame / 60) % 2 == 0)
            ClearBackground(SKYBLUE);
        else
            ClearBackground(LIGHTGRAY);

        int x = frame % w;
        DrawRectangle(x, 30, 50, 50, RED);

        const char *msg = "M5Stack-AtomS3R";

        int sz = 20;
        int tw = MeasureText(msg, sz);
        DrawText(msg, (w - tw) / 2, (h - sz) / 2, sz, BLACK);

        DrawRectangle(0, 0, 20, 20, YELLOW);
        DrawRectangle(w - 20, 0, 20, 20, ORANGE);
        DrawRectangle(0, h - 20, 20, 20, PURPLE);
        DrawRectangle(w - 20, h - 20, 20, 20, PINK);

        EndDrawing();
        frame++;
    }

    CloseWindow();
    vTaskDelete(NULL);
}

void app_main(void)
{
    ESP_LOGI(TAG, "Raylib Demo");

    ESP_ERROR_CHECK(init_display());

    xTaskCreatePinnedToCore(raylib_task, "raylib", RAYLIB_TASK_STACK_SIZE, NULL, 5, NULL, 1);
}
