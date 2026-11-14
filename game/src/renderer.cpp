#include "renderer.hpp"

namespace dino {

Renderer::Renderer(int width, int height, const char* title) : width_(width), height_(height) {
    InitWindow(width, height, title);
    SetTargetFPS(60);

    // Load textures (assuming you place your PNGs in a folder called 'assets')
    tex_dino_run1 = LoadTexture("assets/dino_run1.png");
    tex_dino_run2 = LoadTexture("assets/dino_run2.png");
    tex_dino_duck1 = LoadTexture("assets/dino_duck1.png");
    tex_dino_duck2 = LoadTexture("assets/dino_duck2.png");
    tex_cactus_small = LoadTexture("assets/cactus_small.png");
    tex_cactus_large = LoadTexture("assets/cactus_large.png");
    tex_bird1 = LoadTexture("assets/bird1.png");
    tex_bird2 = LoadTexture("assets/bird2.png");
}

Renderer::~Renderer() {
    // Free GPU VRAM
    UnloadTexture(tex_dino_run1);
    UnloadTexture(tex_dino_run2);
    UnloadTexture(tex_dino_duck1);
    UnloadTexture(tex_dino_duck2);
    UnloadTexture(tex_cactus_small);
    UnloadTexture(tex_cactus_large);
    UnloadTexture(tex_bird1);
    UnloadTexture(tex_bird2);
    
    CloseWindow();
}

bool Renderer::should_close() const {
    return WindowShouldClose();
}

void Renderer::draw_state(const DinoState& state, bool is_game_over) const {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    const float ground_y = 380.0f;
    DrawLine(0, ground_y, width_, ground_y, BLACK);

    // 1. Calculate Animation Frame (oscillates between 0 and 1)
    // We divide ticks_alive by 5 so the legs swap every 5 frames.
    int anim_frame = (state.ticks_alive / 5) % 2;

    // 2. Draw the Player
    float player_h = state.is_ducking ? PLAYER_H_DUCK : PLAYER_H_STAND;
    float screen_player_y = ground_y - state.player_y - player_h;
    
    Texture2D current_dino_tex;
    if (state.player_y > 0.0f) {
        current_dino_tex = tex_dino_run1; // Static frame while jumping
    } else if (state.is_ducking) {
        current_dino_tex = (anim_frame == 0) ? tex_dino_duck1 : tex_dino_duck2;
    } else {
        current_dino_tex = (anim_frame == 0) ? tex_dino_run1 : tex_dino_run2;
    }

    // Draw texture mapped precisely to the physics hitbox
    Rectangle source_rect = { 0.0f, 0.0f, (float)current_dino_tex.width, (float)current_dino_tex.height };
    Rectangle dest_rect = { PLAYER_X, screen_player_y, PLAYER_W, player_h };
    Vector2 origin = { 0.0f, 0.0f };
    
    DrawTexturePro(current_dino_tex, source_rect, dest_rect, origin, 0.0f, WHITE);

    // 3. Draw the Obstacles from the Circular Ring Buffer
    for (int i = 0; i < state.obstacles.count; ++i) {
        const struct Obstacle* obs = queue_get_const(&state.obstacles, i);
        float screen_obs_y = ground_y - obs->y - obs->height;
        
        Texture2D obs_tex;
        if (obs->y > 0.0f) { // Pterodactyl
            obs_tex = (anim_frame == 0) ? tex_bird1 : tex_bird2; // Flapping animation
        } else if (obs->width > 30.0f) {
            obs_tex = tex_cactus_large;
        } else {
            obs_tex = tex_cactus_small;
        }

        Rectangle obs_source = { 0.0f, 0.0f, (float)obs_tex.width, (float)obs_tex.height };
        Rectangle obs_dest = { obs->x, screen_obs_y, obs->width, obs->height };
        DrawTexturePro(obs_tex, obs_source, obs_dest, origin, 0.0f, WHITE);
    }

    // 4. UI Elements
    DrawText(TextFormat("SCORE: %d", state.score), 20, 20, 20, DARKGRAY);
    DrawText(TextFormat("SPEED: %.2f", state.game_speed), 20, 50, 16, GRAY);
    
    if (is_game_over) {
        int over_width = MeasureText("GAME OVER", 40);
        DrawText("GAME OVER", (width_ - over_width) / 2, height_ / 2 - 40, 40, MAROON);
    }

    EndDrawing();
}

} // namespace dino