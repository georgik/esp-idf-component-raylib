#include "raylib_bsp.h"
#include <bsp/esp-bsp.h>

const char* raylib_bsp_get_board_name(void) {
    return "ESP32-P4-Function-EV-Board";
}

int raylib_bsp_get_display_width(void) {
    return BSP_LCD_H_RES;
}

int raylib_bsp_get_display_height(void) {
    return BSP_LCD_V_RES;
}

esp_err_t raylib_bsp_display_init(esp_lcd_panel_handle_t *panel_handle,
                                   esp_lcd_panel_io_handle_t *panel_io_handle) {
    // ESP32-P4 uses MIPI DSI, not SPI, so config structure is different
    bsp_display_config_t config = {
        .hdmi_resolution = BSP_HDMI_RES_NONE,  // Use default LCD, not HDMI
        .dsi_bus = {
            .lane_bit_rate_mbps = BSP_LCD_MIPI_DSI_LANE_BITRATE_MBPS,
        },
    };
    return bsp_display_new(&config, panel_handle, panel_io_handle);
}

esp_err_t raylib_bsp_backlight_on(void) {
    return bsp_display_backlight_on();
}

esp_err_t raylib_bsp_backlight_off(void) {
    return bsp_display_backlight_off();
}
