idf_component_register(
    SRCS
        #"src/raylib_wrapper.c"
        # Core Raylib source files for video rendering
        "raylib/src/rcore.c"
        "raylib/src/rtext.c"
        "raylib/src/rtextures.c"
        "raylib/src/rmodels.c"
        "raylib/src/rshapes.c"
        "raylib/src/utils.c"

        "src/platforms/rcore_esp_idf.c"
    INCLUDE_DIRS
        #"include"
        "raylib/src"
    PRIV_REQUIRES
        esp_lcd
)

# Suppress incompatible pointer type warnings
# Reason: M3D
target_compile_options(${COMPONENT_TARGET} PRIVATE
    -Wno-incompatible-pointer-types  # Suppress pointer type mismatch warnings
    -Wno-uninitialized               # Suppress uninitialized variable warnings
    -Wno-nonnull                     # Suppress non-null argument warnings
    -Wno-return-type                 # Suppress missing or incorrect return type warnings
)

