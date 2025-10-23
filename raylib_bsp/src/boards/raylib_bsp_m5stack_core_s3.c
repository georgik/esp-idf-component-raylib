#include "raylib_bsp.h"
#include <bsp/esp-bsp.h>

const char* raylib_bsp_get_board_name(void) {
    return "M5Stack Core S3";
}

int raylib_bsp_get_display_width(void) {
    return BSP_LCD_H_RES;
}

int raylib_bsp_get_display_height(void) {
    return BSP_LCD_V_RES;
}

esp_err_t raylib_bsp_display_init(esp_lcd_panel_handle_t *panel_handle,
                                   esp_lcd_panel_io_handle_t *panel_io_handle) {
    bsp_display_config_t config = {
        .max_transfer_sz = BSP_LCD_DRAW_BUFF_SIZE * sizeof(uint16_t),
    };
    return bsp_display_new(&config, panel_handle, panel_io_handle);
}

esp_err_t raylib_bsp_backlight_on(void) {
    return bsp_display_backlight_on();
}

esp_err_t raylib_bsp_backlight_off(void) {
    return bsp_display_backlight_off();
}
