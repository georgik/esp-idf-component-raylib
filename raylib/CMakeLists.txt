idf_component_register(
    SRCS
        #"src/raylib_wrapper.c"
        # Core Raylib source files for video rendering
        "raylib/src/rcore.c"
        "raylib/src/rtextures.c"
        "raylib/src/rmodels.c"
        "raylib/src/utils.c"
    INCLUDE_DIRS
        #"include"
        "raylib/src"
    PRIV_REQUIRES
        esp_lcd
)

