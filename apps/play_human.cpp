// game/src/play_human.cpp
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
    // Generate a unique CPU seed based on current wall-clock time
    uint32_t seed = static_cast<uint32_t>(std::time(nullptr));
    if (seed == 0) seed = 1; // Prevent zero seed
    dino::reset_env(state, seed);

    bool is_game_over = false;
    float reward = 0.0f; // Human doesn't need this, but step_env requires it

    // 3. Main Game Loop (capped at 60 FPS by Renderer's SetTargetFPS)
    while (!renderer.should_close()) {
        
        if (is_game_over) {
            // Wait for human to press SPACE to restart
            if (IsKeyPressed(KEY_SPACE)) {
                seed++; // Slightly alter seed for a new obstacle pattern
                dino::reset_env(state, seed);
                is_game_over = false;
            }
        } else {
            // Map Keyboard to Integer Action
            int action = 0; // Default: Run
            
            // Prioritize ducking. If duck is held, jumping is disabled.
            if (IsKeyDown(KEY_DOWN)) {
                action = 2; // Duck
            } else if (IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_UP)) {
                action = 1; // Jump
            }

            // Step the math forward by exactly one discrete frame
            dino::step_env(state, action, reward, is_game_over);
        }

        // 4. Draw the calculated state to the screen
        renderer.draw_state(state, is_game_over);
    }

    return 0;
}