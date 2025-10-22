/**********************************************************************************************
*
*   rlsw_esp_idf.h - Override rlsw memory allocators and configure for ESP-IDF
*
*   This header must be included BEFORE rlsw.h to override defaults
*
**********************************************************************************************/

#ifndef RLSW_ESP_IDF_H
#define RLSW_ESP_IDF_H

#include "esp_heap_caps.h"

// Override rlsw memory allocators to use PSRAM for large framebuffer allocations
#define SW_MALLOC(sz) heap_caps_malloc(sz, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)
#define SW_FREE(ptr) heap_caps_free(ptr)

// Configure framebuffer copy format for RGB (not BGR)
// ESP-Box-3 LCD expects RGB565 format
#define SW_GL_FRAMEBUFFER_COPY_BGRA false

#endif // RLSW_ESP_IDF_H
