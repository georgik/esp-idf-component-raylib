cmake_minimum_required(VERSION 3.16)

# Raylib BSP Abstraction Layer Component
# Conditional BSP selection based on Kconfig

# Temporary workaround: Hardcode BSP dependency to match example manifest
# TODO: Make this work with Kconfig-based conditional selection
set(COMPONENT_SRCS "src/raylib_bsp_common.c" "src/boards/raylib_bsp_esp_box_3.c")
set(COMPONENT_REQUIRES "esp_lcd")
set(COMPONENT_PRIV_REQUIRES "espressif__esp-box-3_noglib")
message(STATUS "Raylib BSP: Hardcoded ESP-BOX-3 for testing")

# Register the component
idf_component_register(
    SRCS ${COMPONENT_SRCS}
    INCLUDE_DIRS "include"
    REQUIRES ${COMPONENT_REQUIRES}
    PRIV_REQUIRES ${COMPONENT_PRIV_REQUIRES}
)
