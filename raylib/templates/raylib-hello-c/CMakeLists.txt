cmake_minimum_required(VERSION 3.16)

# Board-specific display configuration
#IF option("esp32_s3_lcd_ev_board")
set(LCD_H_RES 480)
set(LCD_V_RES 480)
//ELIF option("esp32s3_box_3") || option("esp_box") || option("esp32s3_korvo_1") || option("esp32s3_korvo_2")
set(LCD_H_RES 320)
set(LCD_V_RES 240)
//ENDIF

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32s3_hello)
