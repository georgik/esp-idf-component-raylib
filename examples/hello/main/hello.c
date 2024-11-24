#include "raylib.h"
#include "esp_log.h"

#define TAG "RaylibDemo"

void app_main(void)
{
    // Initialize Raylib with screen dimensions
    const int screenWidth = 240;
    const int screenHeight = 320;

    ESP_LOGI(TAG, "Initializing Raylib...");
    InitWindow(screenWidth, screenHeight, "Raylib + ESP-IDF Demo");

    ESP_LOGI(TAG, "Raylib Initialized. Entering main loop...");

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Begin drawing
        BeginDrawing();

        // Clear the background
        ClearBackground(RAYWHITE);

        // Draw a rectangle
        DrawRectangle(50, 50, 140, 100, BLUE);

        // Draw a circle
        DrawCircle(screenWidth / 2, screenHeight / 2, 40, RED);

        // Draw text
        DrawText("Hello, ESP32!", 10, 10, 20, DARKGRAY);

        // End drawing
        EndDrawing();
    }

    // De-initialize Raylib
    ESP_LOGI(TAG, "De-initializing Raylib...");
    CloseWindow(); // Close window and OpenGL context
}

