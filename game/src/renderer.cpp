#include "renderer.hpp"
#include<raylib.h>

namespace dino {

    Renderer::Renderer(int width, int height, const char* title)
    : width_(width), height_(height) 
    {
        InitWindow(width_, height_, title);
        SetTargetFPS(60);
    }    
    Renderer::~Renderer() {
        CloseWindow();
    }

    bool Renderer :: should_close() const{
        return WindowShouldClose();
    }

    void Renderer::draw_test_frame() const {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Ground line preview
    DrawLine(0, 380, width_, 380, DARKGRAY);
    // Placeholder Dino hitbox preview
    DrawRectangle(80, 320, 40, 60, DARKGRAY);
    
    DrawText("MILESTONE 1: ENGINE SCAFFOLD READY", 20, 20, 20, DARKGREEN);
    DrawText("Press ESC to close window", 20, 50, 16, GRAY);
    DrawFPS(width_ - 100, 20);

    EndDrawing();
}
}