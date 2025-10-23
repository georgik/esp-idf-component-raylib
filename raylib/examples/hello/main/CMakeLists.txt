idf_component_register(
    SRCS
        "hello.c"
        "board_init.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES
        georgik__raylib
        esp_ray_port
)
