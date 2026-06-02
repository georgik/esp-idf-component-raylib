cmake_minimum_required(VERSION 3.16)

# Board-specific display configuration
set(LCD_H_RES 320)
set(LCD_V_RES 240)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32s3_box_3)
