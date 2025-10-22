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
        "include"                # Our wrapper includes (stubs)
        "raylib/src"
        "raylib/src/external"
    PRIV_REQUIRES
        esp_lcd
        espressif__esp-box-3_noglib
)

# Enable software renderer
target_compile_definitions(${COMPONENT_TARGET} PUBLIC
    GRAPHICS_API_OPENGL_11_SOFTWARE
    PLATFORM_CUSTOM
    SW_COLOR_BUFFER_BITS=16
    SW_DEPTH_BUFFER_BITS=16
)

# Disable features not needed/available on embedded platform
target_compile_definitions(${COMPONENT_TARGET} PRIVATE
    SUPPORT_MODULE_RAUDIO=0          # Disable audio module
    SUPPORT_MODULE_RMODELS=0         # Disable 3D models module (requires file system)
    SUPPORT_SCREEN_CAPTURE=0         # Disable screen capture
    SUPPORT_GIF_RECORDING=0          # Disable GIF recording
    SUPPORT_COMPRESSION_API=0        # Disable compression API
    SUPPORT_AUTOMATION_EVENTS=0      # Disable automation events
)

# Force include our PSRAM allocator overrides before rlsw.h
target_compile_options(${COMPONENT_TARGET} PRIVATE
    -include ${CMAKE_CURRENT_LIST_DIR}/include/rlsw_esp_idf.h
)

# Suppress incompatible pointer type warnings
# Reason: M3D and software renderer
target_compile_options(${COMPONENT_TARGET} PRIVATE
    -Wno-incompatible-pointer-types  # Suppress pointer type mismatch warnings
    -Wno-uninitialized               # Suppress uninitialized variable warnings
    -Wno-nonnull                     # Suppress non-null argument warnings
    -Wno-return-type                 # Suppress missing or incorrect return type warnings
    -Wno-unused-variable             # Suppress unused variable warnings
    -Wno-unused-function             # Suppress unused function warnings
)

