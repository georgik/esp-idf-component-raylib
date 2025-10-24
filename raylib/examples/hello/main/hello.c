#include "raylib.h"
#include "board_init.h"
#include "esp_raylib_port.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "RaylibDemo"
#define RAYLIB_TASK_STACK_SIZE (128 * 1024)  // 128KB stack for software renderer

void raylib_task(void *pvParameter)
{
    // Query actual display dimensions from port
    uint16_t screenWidth = 320;  // Default fallback
    uint16_t screenHeight = 240;
    
    esp_err_t ret = ray_port_get_dimensions(&screenWidth, &screenHeight);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to get display dimensions, using defaults");
    }

    ESP_LOGI(TAG, "Initializing Raylib with display dimensions: %dx%d...", screenWidth, screenHeight);
    InitWindow(screenWidth, screenHeight, "Raylib + ESP-IDF Demo");

    ESP_LOGI(TAG, "Raylib Initialized. Entering main loop...");
    ESP_LOGI(TAG, "Screen dimensions: %dx%d", screenWidth, screenHeight);

    // Color test sequence to verify RGB565 mapping
    ESP_LOGI(TAG, "Starting color test: RED");
    BeginDrawing();
    ClearBackground(RED);
    DrawText("RED", 10, 10, 20, WHITE);
    EndDrawing();
    vTaskDelay(pdMS_TO_TICKS(1000));

    ESP_LOGI(TAG, "Color test: GREEN");
    BeginDrawing();
    ClearBackground(GREEN);
    DrawText("GREEN", 10, 10, 20, WHITE);
    EndDrawing();
    vTaskDelay(pdMS_TO_TICKS(1000));

    ESP_LOGI(TAG, "Color test: BLUE");
    BeginDrawing();
    ClearBackground(BLUE);
    DrawText("BLUE", 10, 10, 20, WHITE);
    EndDrawing();
    vTaskDelay(pdMS_TO_TICKS(1000));

    ESP_LOGI(TAG, "Color test complete. Starting animation...");

    // Main game loop
    int frameCounter = 0;
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Begin drawing
        BeginDrawing();

        // Clear the entire background with alternating colors
        if ((frameCounter / 60) % 2 == 0)
            ClearBackground(SKYBLUE);
        else
            ClearBackground(LIGHTGRAY);

        // Draw a simple moving rectangle
        int x = (frameCounter % screenWidth);
        int y = 30;
        DrawRectangle(x, y, 50, 50, RED);

        // Draw centered text with black color for readability
        const char *message = "Hello ESP32!";
        int fontSize = 30;
        int textWidth = MeasureText(message, fontSize);
        int textX = (screenWidth - textWidth) / 2;
        int textY = (screenHeight - fontSize) / 2;
        DrawText(message, textX, textY, fontSize, BLACK);
        
        // Draw position markers to debug coordinate system
        DrawCircle(0, 0, 10, YELLOW);                    // Top-left origin
        DrawCircle(screenWidth-1, 0, 10, ORANGE);        // Top-right
        DrawCircle(0, screenHeight-1, 10, PURPLE);       // Bottom-left
        DrawCircle(screenWidth-1, screenHeight-1, 10, PINK); // Bottom-right

        // End drawing
        EndDrawing();
        frameCounter++;
    }

    // De-initialize Raylib (never reached in embedded mode)
    ESP_LOGI(TAG, "De-initializing Raylib...");
    CloseWindow(); // Close window and OpenGL context
    vTaskDelete(NULL);
}

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing board display...");
    
    // Initialize display hardware and port layer
    esp_err_t ret = board_init_display();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize display: %d", ret);
        return;
    }
    
    ESP_LOGI(TAG, "Creating raylib task with %dKB stack...", RAYLIB_TASK_STACK_SIZE / 1024);
    
    // Create dedicated task for raylib with large stack
    xTaskCreatePinnedToCore(
        raylib_task,              // Task function
        "raylib_task",            // Task name
        RAYLIB_TASK_STACK_SIZE,   // Stack size in bytes
        NULL,                     // Parameters
        5,                        // Priority
        NULL,                     // Task handle
        1                         // Core ID (run on core 1)
    );
}

