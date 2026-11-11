// game/src/renderer.cpp
#include "renderer.hpp"

namespace dino {

Renderer::Renderer(int width, int height, const char* title) : width_(width), height_(height) {
    InitWindow(width, height, title);
    SetTargetFPS(60); // 60 FPS cap for human play
}

Renderer::~Renderer() {
    CloseWindow();
}

bool Renderer::should_close() const {
    return WindowShouldClose();
}

void Renderer::draw_state(const DinoState& state, bool is_game_over) const {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // The visual ground line
    const float ground_y = 380.0f;
    DrawLine(0, ground_y, width_, ground_y, BLACK);

    // 1. Draw the Player
    float player_h = state.is_ducking ? PLAYER_H_DUCK : PLAYER_H_STAND;
    // Translate physics Y (up is positive) to screen Y (down is positive)
    float screen_player_y = ground_y - state.player_y - player_h;
    
    Color dino_color = is_game_over ? RED : DARKGRAY;
    DrawRectangle(PLAYER_X, screen_player_y, PLAYER_W, player_h, dino_color);

    // 2. Draw the Obstacles
    for (int i = 0; i < MAX_OBSTACLES; ++i) {
        if (!state.obstacles[i].active) continue;
        
        const auto& obs = state.obstacles[i];
        float screen_obs_y = ground_y - obs.y - obs.height;
        
        // Color-code for visual debugging: Flying birds are blue, cacti are green
        Color obs_color = (obs.y > 0.0f) ? SKYBLUE : LIME;
        DrawRectangle(obs.x, screen_obs_y, obs.width, obs.height, obs_color);
    }

    // 3. Draw UI & Metrics
    DrawText(TextFormat("SCORE: %d", state.score), 20, 20, 20, DARKGRAY);
    DrawText(TextFormat("SPEED: %.2f", state.game_speed), 20, 50, 16, GRAY);
    DrawFPS(width_ - 100, 20);

    if (is_game_over) {
        int over_width = MeasureText("GAME OVER", 40);
        DrawText("GAME OVER", (width_ - over_width) / 2, height_ / 2 - 40, 40, MAROON);
        
        int restart_width = MeasureText("Press SPACE to Restart", 20);
        DrawText("Press SPACE to Restart", (width_ - restart_width) / 2, height_ / 2 + 10, 20, DARKGRAY);
    }

    EndDrawing();
}

} // namespace dino