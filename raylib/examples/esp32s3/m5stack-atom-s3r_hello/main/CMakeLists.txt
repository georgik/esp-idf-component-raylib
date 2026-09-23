idf_component_register(SRCS "main.c"
                             "backlight.c"
                    INCLUDE_DIRS "."
                    REQUIRES raylib esp_driver_spi esp_lcd
                             espressif__esp_lcd_gc9a01 esp_driver_i2c
                             esp_driver_gpio nvs_flash)
