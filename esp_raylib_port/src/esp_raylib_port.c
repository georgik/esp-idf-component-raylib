/**
 * @file esp_raylib_port.c
 * @brief ESP Raylib Port Layer Implementation
 */

#include "esp_raylib_port.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <string.h>

#ifdef CONFIG_IDF_TARGET_ESP32P4
#include "esp_lcd_mipi_dsi.h"
#endif

static const char *TAG = "ray_port";

// Port state
typedef struct {
    bool initialized;
    ray_port_cfg_t cfg;
    
    // Display
    esp_lcd_panel_handle_t panel;
    esp_lcd_panel_io_handle_t io;
    uint16_t hres;
    uint16_t vres;
    
    // Thread safety
    SemaphoreHandle_t mutex;
    SemaphoreHandle_t lcd_semaphore;  // For P4 DSI sync
    
    // Performance stats
    uint64_t last_flush_us;
    uint32_t flush_count;
    uint64_t total_bytes;
} ray_port_state_t;

static ray_port_state_t s_port = {0};

esp_err_t ray_port_init(const ray_port_cfg_t* cfg)
{
    if (!cfg) {
        ESP_LOGE(TAG, "Configuration is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (s_port.initialized) {
        ESP_LOGW(TAG, "Port already initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    memcpy(&s_port.cfg, cfg, sizeof(ray_port_cfg_t));
    
    // Create mutex for thread safety
    s_port.mutex = xSemaphoreCreateMutex();
    if (!s_port.mutex) {
        ESP_LOGE(TAG, "Failed to create mutex");
        return ESP_ERR_NO_MEM;
    }
    
    s_port.initialized = true;
    s_port.flush_count = 0;
    s_port.total_bytes = 0;
    
    ESP_LOGI(TAG, "Port initialized: %dx%d, PSRAM=%d, swap_bytes=%d, perf_stats=%d",
             cfg->hres, cfg->vres, cfg->buff_psram, cfg->swap_rgb_bytes, cfg->perf_stats);
    
    return ESP_OK;
}

#ifdef CONFIG_IDF_TARGET_ESP32P4
// DSI panel callback for P4
static bool lcd_event_callback(esp_lcd_panel_handle_t panel, esp_lcd_dpi_panel_event_data_t *edata, void *user_ctx)
{
    BaseType_t high_task_awoken = pdFALSE;
    if (s_port.lcd_semaphore) {
        xSemaphoreGiveFromISR(s_port.lcd_semaphore, &high_task_awoken);
    }
    return high_task_awoken == pdTRUE;
}
#else
// SPI panel callback for non-P4
static bool lcd_event_callback(esp_lcd_panel_io_handle_t io, esp_lcd_panel_io_event_data_t *event_data, void *user_ctx)
{
    BaseType_t high_task_awoken = pdFALSE;
    if (s_port.lcd_semaphore) {
        xSemaphoreGiveFromISR(s_port.lcd_semaphore, &high_task_awoken);
    }
    return high_task_awoken == pdTRUE;
}
#endif

esp_err_t ray_port_add_display(const ray_port_display_cfg_t* disp)
{
    if (!s_port.initialized) {
        ESP_LOGE(TAG, "Port not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (!disp || !disp->panel) {
        ESP_LOGE(TAG, "Invalid display configuration");
        return ESP_ERR_INVALID_ARG;
    }
    
    ray_port_lock(portMAX_DELAY);
    
    s_port.panel = disp->panel;
    s_port.io = disp->io;
    s_port.hres = disp->hres;
    s_port.vres = disp->vres;
    
    // Create semaphore for LCD synchronization
    s_port.lcd_semaphore = xSemaphoreCreateBinary();
    if (!s_port.lcd_semaphore) {
        ray_port_unlock();
        ESP_LOGE(TAG, "Failed to create LCD semaphore");
        return ESP_ERR_NO_MEM;
    }
    
    // Register callback for display transfer completion
#ifdef CONFIG_IDF_TARGET_ESP32P4
    esp_lcd_dpi_panel_event_callbacks_t cbs = {
        .on_color_trans_done = lcd_event_callback,
    };
    esp_lcd_dpi_panel_register_event_callbacks(s_port.panel, &cbs, NULL);
#else
    if (s_port.io) {
        esp_lcd_panel_io_callbacks_t cbs = {
            .on_color_trans_done = lcd_event_callback,
        };
        esp_lcd_panel_io_register_event_callbacks(s_port.io, &cbs, NULL);
    }
#endif
    
    ray_port_unlock();
    
    ESP_LOGI(TAG, "Display registered: %dx%d, DMA=%d", disp->hres, disp->vres, disp->dma_capable);
    
    return ESP_OK;
}

esp_err_t ray_port_remove_display(void)
{
    if (!s_port.initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    ray_port_lock(portMAX_DELAY);
    
    s_port.panel = NULL;
    s_port.io = NULL;
    s_port.hres = 0;
    s_port.vres = 0;
    
    ray_port_unlock();
    
    ESP_LOGI(TAG, "Display removed");
    
    return ESP_OK;
}

esp_err_t ray_port_add_touch(const ray_port_touch_cfg_t* touch_cfg)
{
    ESP_LOGW(TAG, "Touch input not yet implemented");
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t ray_port_backlight_on(void)
{
    ESP_LOGW(TAG, "Backlight control should be handled by application");
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t ray_port_backlight_off(void)
{
    ESP_LOGW(TAG, "Backlight control should be handled by application");
    return ESP_ERR_NOT_SUPPORTED;
}

void ray_port_lock(uint32_t timeout_ms)
{
    if (s_port.mutex) {
        TickType_t ticks = (timeout_ms == 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
        xSemaphoreTake(s_port.mutex, ticks);
    }
}

void ray_port_unlock(void)
{
    if (s_port.mutex) {
        xSemaphoreGive(s_port.mutex);
    }
}

esp_err_t ray_port_flush_rgb565(const uint16_t* buf, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    if (!s_port.initialized || !s_port.panel) {
        ESP_LOGE(TAG, "Port or display not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (!buf) {
        ESP_LOGE(TAG, "Buffer is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    uint64_t start_us = esp_timer_get_time();
    
    // Calculate chunk size
    size_t chunk_bytes = s_port.cfg.chunk_bytes;
    if (chunk_bytes == 0) {
        // Auto: max 48 lines per chunk (matches ESP-BSP pattern)
        chunk_bytes = w * 48 * sizeof(uint16_t);
    }
    
    int max_chunk_height = chunk_bytes / (w * sizeof(uint16_t));
    if (max_chunk_height == 0) max_chunk_height = 1;
    
    // Byte swap if needed
    uint16_t *swap_buf = NULL;
    const uint16_t *flush_buf = buf;
    
    if (s_port.cfg.swap_rgb_bytes) {
        size_t total_pixels = w * h;
        swap_buf = malloc(total_pixels * sizeof(uint16_t));
        if (!swap_buf) {
            ESP_LOGE(TAG, "Failed to allocate swap buffer");
            return ESP_ERR_NO_MEM;
        }
        
        for (size_t i = 0; i < total_pixels; i++) {
            uint16_t pixel = buf[i];
            swap_buf[i] = (pixel >> 8) | (pixel << 8);
        }
        
        flush_buf = swap_buf;
    }
    
    // Flush in chunks
    esp_err_t ret = ESP_OK;
    for (int row = 0; row < h; row += max_chunk_height) {
        int chunk_height = (row + max_chunk_height > h) ? (h - row) : max_chunk_height;
        const uint16_t *chunk_pixels = flush_buf + (row * w);
        
        ret = esp_lcd_panel_draw_bitmap(s_port.panel, x, y + row, x + w, y + row + chunk_height, (void *)chunk_pixels);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to draw bitmap chunk at row %d: %s", row, esp_err_to_name(ret));
            break;
        }
        
        // Wait for transfer to complete (important for DSI/DPI panels on P4)
        if (s_port.lcd_semaphore) {
            xSemaphoreTake(s_port.lcd_semaphore, portMAX_DELAY);
        }
    }
    
    if (swap_buf) {
        free(swap_buf);
    }
    
    // Performance stats
    if (s_port.cfg.perf_stats) {
        uint64_t elapsed_us = esp_timer_get_time() - start_us;
        s_port.flush_count++;
        s_port.total_bytes += w * h * sizeof(uint16_t);
        
        if (s_port.flush_count % 60 == 0) {  // Log every 60 frames
            float fps = 1000000.0f / (float)elapsed_us;
            float bytes_per_sec = (float)s_port.total_bytes / ((float)(esp_timer_get_time() - s_port.last_flush_us) / 1000000.0f);
            ESP_LOGI(TAG, "Flush #%lu: %dx%d in %.2f ms (%.1f FPS, %.2f KB/s)",
                     s_port.flush_count, w, h, elapsed_us / 1000.0f, fps, bytes_per_sec / 1024.0f);
            s_port.total_bytes = 0;
            s_port.last_flush_us = esp_timer_get_time();
        }
    }
    
    return ret;
}

esp_err_t ray_port_get_dimensions(uint16_t* w, uint16_t* h)
{
    if (!s_port.initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (w) *w = s_port.hres;
    if (h) *h = s_port.vres;
    
    return ESP_OK;
}

esp_err_t ray_port_deinit(void)
{
    if (!s_port.initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    ray_port_lock(portMAX_DELAY);
    
    s_port.panel = NULL;
    s_port.io = NULL;
    s_port.initialized = false;
    
    if (s_port.lcd_semaphore) {
        vSemaphoreDelete(s_port.lcd_semaphore);
        s_port.lcd_semaphore = NULL;
    }
    
    if (s_port.mutex) {
        vSemaphoreDelete(s_port.mutex);
        s_port.mutex = NULL;
    }
    
    ESP_LOGI(TAG, "Port deinitialized");
    
    return ESP_OK;
}
