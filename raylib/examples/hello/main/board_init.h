/**
 * @file board_init.h
 * @brief Board initialization interface
 */

#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize display hardware and raylib port layer
 * 
 * This function handles board-specific initialization including:
 * - Creating esp_lcd panel/io handles
 * - Initializing backlight
 * - Configuring raylib port layer
 * - Registering display with port
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t board_init_display(void);

#ifdef __cplusplus
}
#endif
