#pragma once

#include "esp_lcd_types.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// Raylib BSP Abstraction API
// This provides a board-agnostic interface to ESP-BSP functionality

/**
 * @brief Get the board name
 * @return Board name string
 */
const char* raylib_bsp_get_board_name(void);

/**
 * @brief Get display width
 * @return Display width in pixels
 */
int raylib_bsp_get_display_width(void);

/**
 * @brief Get display height  
 * @return Display height in pixels
 */
int raylib_bsp_get_display_height(void);

/**
 * @brief Initialize display
 * @param panel_handle Pointer to store panel handle
 * @param panel_io_handle Pointer to store panel IO handle
 * @return ESP_OK on success
 */
esp_err_t raylib_bsp_display_init(esp_lcd_panel_handle_t *panel_handle, 
                                   esp_lcd_panel_io_handle_t *panel_io_handle);

/**
 * @brief Turn backlight on
 * @return ESP_OK on success
 */
esp_err_t raylib_bsp_backlight_on(void);

/**
 * @brief Turn backlight off
 * @return ESP_OK on success
 */
esp_err_t raylib_bsp_backlight_off(void);

#ifdef __cplusplus
}
#endif
