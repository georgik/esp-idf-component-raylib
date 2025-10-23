cmake_minimum_required(VERSION 3.16)

# Raylib BSP Abstraction Layer Component
# Conditional BSP selection based on IDF_TARGET

set(COMPONENT_SRCS "src/raylib_bsp_common.c")
set(COMPONENT_REQUIRES "esp_lcd")
set(COMPONENT_PRIV_REQUIRES "")

# Get all available components
idf_build_get_property(build_components BUILD_COMPONENTS)

# Detect target and add appropriate board implementation
# BSP dependencies are managed through main/idf_component.yml conditionally
if(IDF_TARGET STREQUAL "esp32p4")
    set(BSP_COMPONENT "espressif__esp32_p4_function_ev_board_noglib")
    list(APPEND COMPONENT_SRCS "src/boards/raylib_bsp_esp32_p4_function_ev.c")
    # Only add BSP if it exists in the build
    if("${BSP_COMPONENT}" IN_LIST build_components)
        list(APPEND COMPONENT_PRIV_REQUIRES "${BSP_COMPONENT}")
        message(STATUS "Raylib BSP: Selected ESP32-P4 Function EV Board (noglib)")
    else()
        message(WARNING "Raylib BSP: BSP component ${BSP_COMPONENT} not found, using fallback")
    endif()
elseif(IDF_TARGET STREQUAL "esp32s3")
    set(BSP_COMPONENT "espressif__esp-box-3_noglib")
    list(APPEND COMPONENT_SRCS "src/boards/raylib_bsp_esp_box_3.c")
    # Only add BSP if it exists in the build
    if("${BSP_COMPONENT}" IN_LIST build_components)
        list(APPEND COMPONENT_PRIV_REQUIRES "${BSP_COMPONENT}")
        message(STATUS "Raylib BSP: Selected ESP32-S3-BOX-3")
    else()
        message(WARNING "Raylib BSP: BSP component ${BSP_COMPONENT} not found, using fallback")
    endif()
else()
    message(FATAL_ERROR "Raylib BSP: Unsupported target ${IDF_TARGET}")
endif()

# Register the component
idf_component_register(
    SRCS ${COMPONENT_SRCS}
    INCLUDE_DIRS "include"
    REQUIRES ${COMPONENT_REQUIRES}
    PRIV_REQUIRES ${COMPONENT_PRIV_REQUIRES}
)
