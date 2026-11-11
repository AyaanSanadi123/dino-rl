#pragma once
#include <cstdint>
#include "compat.hpp"

namespace dino {
    // Physics Constants
    constexpr float GRAVITY = -0.85f;
    constexpr float JUMP_VELOCITY = 14.5f;
    constexpr float PLAYER_X = 80.0f;
    constexpr float PLAYER_W = 40.0f;
    constexpr float PLAYER_H_STAND = 60.0f;
    constexpr float PLAYER_H_DUCK = 30.0f;
    constexpr float MAX_SPEED = 16.0f;
    constexpr int MAX_OBSTACLES = 5; // Upgraded from 3 to 5 for denser multi-obstacle RL scenarios

    // define an obstacle 
    struct Obstacle {
        float x;
        float y;
        float width;
        float height;
        bool active;
    };
    struct DinoState{
        float player_y; // this is the vertical position of the dinosaur
        float player_vy; // this is the vertical velocity of the dino, reduces with time as gravity acts 
        bool is_ducking;

        float game_speed;
        int score;
        int ticks_alive; // this is the amount of frames the player has been alive, use to calculate score 

        uint32_t rng_state; // this is a random number generator, that works on both cpu and gpu indifferently 
        Obstacle obstacles[MAX_OBSTACLES];
    };
    
      // Extremely fast, hardware-agnostic random number generator
    HOST_DEVICE FORCE_INLINE uint32_t xorshift32(uint32_t* state) {
        uint32_t x = *state;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        *state = x;
        return x;
    }

    // Returns float between 0.0 and 1.0
    HOST_DEVICE FORCE_INLINE float rand_float(uint32_t* state) {
        return (float)(xorshift32(state) & 0xFFFFFF) / 16777216.0f;
    }

    HOST_DEVICE FORCE_INLINE void reset_env(DinoState& state, uint32_t seed) {
        state.player_y = 0.0f;
        state.player_vy = 0.0f;
        state.is_ducking = false;
        state.game_speed = 7.0f;
        state.score = 0;
        state.ticks_alive = 0;
        state.rng_state = (seed == 0) ? 123456789 : seed; // Seed cannot be 0

        for (int i = 0; i < MAX_OBSTACLES; ++i) {
            state.obstacles[i].active = false;
        }
    }

    HOST_DEVICE FORCE_INLINE bool check_collision(const DinoState& state){
        float player_h = state.is_ducking ? PLAYER_H_DUCK : PLAYER_H_STAND;
        
        // Add a small padding to make hitboxes forgiving against transparent sprite padding
        float padding = 8.0f;

        for (int i = 0; i < MAX_OBSTACLES; ++i) {
            if (!state.obstacles[i].active) continue;
            const auto &obs = state.obstacles[i];

            bool overlap_x = (PLAYER_X + padding < obs.x + obs.width - padding) && 
                            (PLAYER_X + PLAYER_W - padding > obs.x + padding);
                            
            bool overlap_y = (state.player_y < obs.y + obs.height - padding) && 
                            (state.player_y + player_h > obs.y + padding);
        
            if (overlap_x && overlap_y) return true;
        }
        return false;
    }

    // things that need to happen after every frame passes 
    HOST_DEVICE FORCE_INLINE void step_env(DinoState& state, int action, float& reward, bool& done){
        reward = 0.05f; // Base survival reward
        done = false;
        state.ticks_alive++;
        
        // process the action 
        state.is_ducking = (action == 2 && state.player_y == 0.0f); // if the player is ducking, then action must be 2 and vertical elevation must be 0

        if(action == 1 && state.player_y == 0.0f) state.player_vy = JUMP_VELOCITY;

        // 2. Player Physics
        float grav_multiplier = (action == 2 && state.player_y > 0.0f) ? 2.8f : 1.0f;
        state.player_y += state.player_vy; // move the dino upwards wrt its current velocity
        state.player_vy += GRAVITY * grav_multiplier;

        if (state.player_y <= 0.0f) {
            state.player_y = 0.0f;
            state.player_vy = 0.0f;
        }

        // 3. Move & Despawn Obstacles
        float furthest_x = 0.0f;
        for (int i = 0; i < MAX_OBSTACLES; ++i) {
            if (!state.obstacles[i].active) continue;
            
            float old_x = state.obstacles[i].x;
            state.obstacles[i].x -= state.game_speed;
            
            // Reward for clearing an obstacle
            if (old_x >= PLAYER_X && state.obstacles[i].x < PLAYER_X) {
                reward += 1.0f;
                state.score += 100;
            }

            if (state.obstacles[i].x + state.obstacles[i].width < 0) {
                state.obstacles[i].active = false;
            } else if (state.obstacles[i].x > furthest_x) {
                furthest_x = state.obstacles[i].x;
            }
        }

        // 4. Speed & Spawning Logic
        if (state.game_speed < MAX_SPEED) {
            state.game_speed += 0.002f;
        }
        
        float min_gap = 200.0f + (state.game_speed * 12.0f); // Slightly adjusted spacing for 5 obstacles

        if (furthest_x < 800.0f - min_gap) {
            // Find inactive slot
            for (int i = 0; i < MAX_OBSTACLES; ++i) {
                if (!state.obstacles[i].active) {
                    state.obstacles[i].active = true;
                    state.obstacles[i].x = 800.0f + rand_float(&state.rng_state) * 200.0f;
                    
                    float r = rand_float(&state.rng_state);
                    if (r < 0.25f) {
                        // 1. Small ground cactus (Jump over)
                        state.obstacles[i].y = 0.0f;
                        state.obstacles[i].width = 25.0f;
                        state.obstacles[i].height = 45.0f;
                    } else if (r < 0.50f) {
                        // 2. Large ground cactus cluster (Jump over)
                        state.obstacles[i].y = 0.0f;
                        state.obstacles[i].width = 50.0f;
                        state.obstacles[i].height = 60.0f;
                    } else if (r < 0.75f) {
                        // 3. Walk-under bird (High altitude - run right under it)
                        state.obstacles[i].y = 65.0f;
                        state.obstacles[i].width = 45.0f;
                        state.obstacles[i].height = 25.0f;
                    } else {
                        // 4. Duck-under bird (Mid altitude - MUST duck)
                        state.obstacles[i].y = 35.0f;
                        state.obstacles[i].width = 45.0f;
                        state.obstacles[i].height = 30.0f;
                    }
                    break;
                }
            }
        }

        // 5. Terminal State Check
        if (check_collision(state)) {
            reward = -10.0f;
            done = true;
        }
    }
}