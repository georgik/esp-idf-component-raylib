idf_component_register(
    SRCS
        "hello.c"
        "board_init.c"
    INCLUDE_DIRS "."
    REQUIRES
        esp_adc  # Required by M5Stack Core2 BSP for audio codec - must be public for esp_codec_dev
    PRIV_REQUIRES
        georgik__raylib
        # esp_raylib_port is pulled in via georgik__raylib dependency
)
