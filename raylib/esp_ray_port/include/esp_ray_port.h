/**
 * @file esp_ray_port.h
 * @brief ESP Raylib Port Layer - Board-agnostic display/touch abstraction
 * 
 * This component provides a stable port layer API between raylib and ESP-IDF
 * display/touch drivers (esp_lcd, esp_lcd_touch). Applications create the
 * panel/touch handles and pass them to the port, which handles framebuffer
 * management, flush, and optional input.
 * 
 * @copyright Copyright (c) 2025
 * @license Zlib
 */

#pragma once

#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Port configuration options
 */
typedef struct {
    bool buff_psram;          ///< Allocate framebuffer in PSRAM if available
    bool double_buffer;       ///< Enable double buffering (future use)
    size_t buffer_pixels;     ///< Buffer size in pixels (0 = full screen)
    size_t chunk_bytes;       ///< Max chunk size for SPI transfer (0 = auto)
    bool swap_rgb_bytes;      ///< Swap byte order for RGB565 (big/little endian)
    uint16_t hres;            ///< Horizontal resolution
    uint16_t vres;            ///< Vertical resolution
    int rotation;             ///< Display rotation (0, 90, 180, 270)
    bool perf_stats;          ///< Enable performance statistics logging
} ray_port_cfg_t;

/**
 * @brief Display configuration
 */
typedef struct {
    esp_lcd_panel_handle_t panel;      ///< ESP-LCD panel handle (required)
    esp_lcd_panel_io_handle_t io;      ///< ESP-LCD panel IO handle (optional)
    uint16_t hres;                      ///< Horizontal resolution
    uint16_t vres;                      ///< Vertical resolution
    bool monochrome;                    ///< Monochrome display (not yet supported)
    bool dma_capable;                   ///< Display supports DMA transfers
} ray_port_display_cfg_t;

/**
 * @brief Touch configuration (future use)
 */
typedef struct {
    void *touch;              ///< ESP-LCD touch handle (esp_lcd_touch_handle_t)
    float scale_x;            ///< X-axis scaling factor
    float scale_y;            ///< Y-axis scaling factor
} ray_port_touch_cfg_t;

/**
 * @brief Initialize the port layer
 * 
 * Must be called before any other port functions.
 * 
 * @param cfg Port configuration
 * @return ESP_OK on success
 */
esp_err_t ray_port_init(const ray_port_cfg_t* cfg);

/**
 * @brief Register display panel with the port
 * 
 * The panel must be already initialized by the application.
 * 
 * @param disp Display configuration
 * @return ESP_OK on success
 */
esp_err_t ray_port_add_display(const ray_port_display_cfg_t* disp);

/**
 * @brief Unregister display panel
 * 
 * @return ESP_OK on success
 */
esp_err_t ray_port_remove_display(void);

/**
 * @brief Register touch input (not yet implemented)
 * 
 * @param touch_cfg Touch configuration
 * @return ESP_OK on success
 */
esp_err_t ray_port_add_touch(const ray_port_touch_cfg_t* touch_cfg);

/**
 * @brief Turn backlight on (requires GPIO control from app)
 * 
 * @return ESP_OK on success
 */
esp_err_t ray_port_backlight_on(void);

/**
 * @brief Turn backlight off (requires GPIO control from app)
 * 
 * @return ESP_OK on success
 */
esp_err_t ray_port_backlight_off(void);

/**
 * @brief Lock port for thread-safe operations
 * 
 * @param timeout_ms Timeout in milliseconds (0 = wait forever)
 */
void ray_port_lock(uint32_t timeout_ms);

/**
 * @brief Unlock port
 */
void ray_port_unlock(void);

/**
 * @brief Flush RGB565 framebuffer to display
 * 
 * Transfers the specified rectangle to the display in chunks.
 * 
 * @param buf RGB565 framebuffer
 * @param x X coordinate (left)
 * @param y Y coordinate (top)
 * @param w Width in pixels
 * @param h Height in pixels
 * @return ESP_OK on success
 */
esp_err_t ray_port_flush_rgb565(const uint16_t* buf, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

/**
 * @brief Get display dimensions
 * 
 * @param w Output horizontal resolution
 * @param h Output vertical resolution
 * @return ESP_OK on success
 */
esp_err_t ray_port_get_dimensions(uint16_t* w, uint16_t* h);

/**
 * @brief Deinitialize the port layer
 * 
 * @return ESP_OK on success
 */
esp_err_t ray_port_deinit(void);

#ifdef __cplusplus
}
#endif
