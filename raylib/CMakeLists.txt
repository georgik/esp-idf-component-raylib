idf_component_register(
    SRCS
        # Core Raylib source files (minimal set for 2D rendering)
        "raylib/src/rcore.c"
        "raylib/src/rtext.c"
        "raylib/src/rtextures.c"
        "raylib/src/rshapes.c"
        "raylib/src/utils.c"
        # NOTE: rmodels.c excluded - requires file system APIs not available on embedded

        # Platform-specific implementation
        "src/platforms/rcore_esp_idf.c"
    INCLUDE_DIRS
        "include"                # Our wrapper includes (stubs) - custom config.h here takes precedence
        "raylib/src"
        "raylib/src/external"
    REQUIRES
        esp_raylib_port  # Port layer abstraction - board-agnostic display/touch API
    PRIV_REQUIRES
        esp_lcd
)

# Enable software renderer
target_compile_definitions(${COMPONENT_TARGET} PUBLIC
    GRAPHICS_API_OPENGL_11_SOFTWARE
    PLATFORM_CUSTOM
    SW_COLOR_BUFFER_BITS=16
    SW_DEPTH_BUFFER_BITS=16
)

# Prioritize our custom include directory for config.h lookup and suppress warnings
target_compile_options(${COMPONENT_TARGET} PRIVATE
    -I${CMAKE_CURRENT_LIST_DIR}/include          # Custom config.h takes precedence
    -Wno-incompatible-pointer-types              # M3D and software renderer compatibility
    -Wno-uninitialized
    -Wno-nonnull
    -Wno-return-type
    -Wno-unused-variable
    -Wno-unused-function
    -Wno-macro-redefined                         # SW_MALLOC/SW_FREE redefined for PSRAM
    -Wno-stringop-truncation                     # Raylib rtext.c TextReplaceBetween
)

