/*
 * AtomS3R LCD backlight (LP5562 I2C LED driver) control.
 */

#ifndef BACKLIGHT_H
#define BACKLIGHT_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the I2C bus + LP5562 driver and turn the backlight on.
 *
 * @return ESP_OK on success.
 */
esp_err_t backlight_init(void);

/**
 * @brief Set backlight brightness (0..100 percent) and ensure it is enabled.
 *
 * @param brightness_percent 0 (off) .. 100 (full).
 * @return ESP_OK on success.
 */
esp_err_t backlight_set_brightness(int brightness_percent);

/** @brief Drive the backlight at full brightness. */
void backlight_on(void);

/** @brief Turn the backlight off. */
void backlight_off(void);

#ifdef __cplusplus
}
#endif

#endif /* BACKLIGHT_H */
