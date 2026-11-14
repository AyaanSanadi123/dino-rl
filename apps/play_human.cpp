// apps/play_human.cpp
#include <iostream>
#include <ctime>
#include <raylib.h>
#include "renderer.hpp"
#include "dino_physics.hpp"

int main() {
    // 1. Initialize Window and Renderer
    dino::Renderer renderer(800, 450, "Dino CUDA-RL - Human Play");

    // 2. Initialize Physics State
    dino::DinoState state;
    uint32_t seed = static_cast<uint32_t>(std::time(nullptr));
    if (seed == 0) seed = 1;
    dino::reset_env(state, seed);

    bool is_game_over = false;

    // 3. Main Game Loop (capped at 60 FPS by Renderer's SetTargetFPS)
    while (!renderer.should_close()) {
        if (is_game_over) {
            if (IsKeyPressed(KEY_SPACE)) {
                seed++;
                dino::reset_env(state, seed);
                is_game_over = false;
            }
        } else {
            int action = 0; // Default: Run
            
            if (IsKeyDown(KEY_DOWN)) {
                action = 2; // Duck
            } else if (IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_UP)) {
                action = 1; // Jump
            }

            // Pure physics step
            dino::step_env(state, action, is_game_over);
        }

        // 4. Draw the calculated state to the screen
        renderer.draw_state(state, is_game_over);
    }

    return 0;
}