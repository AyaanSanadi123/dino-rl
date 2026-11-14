// dino_physics.hpp
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "compat.hpp"
#include "obstacle_queue.hpp"

namespace dino {
    // Physics Constants
    constexpr float GRAVITY = -0.85f;
    constexpr float JUMP_VELOCITY = 14.5f;
    constexpr float PLAYER_X = 80.0f;
    constexpr float PLAYER_W = 40.0f;
    constexpr float PLAYER_H_STAND = 60.0f;
    constexpr float PLAYER_H_DUCK = 30.0f;
    constexpr float MAX_SPEED = 16.0f;

    struct DinoState {
        float player_y;
        float player_vy;
        bool is_ducking;

        float game_speed;
        int score;
        int ticks_alive;

        uint32_t rng_state;
        struct ObstacleQueue obstacles; // Change 1: Ring buffer replaces raw array
    };
    
    // Hardware-agnostic random number generator
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

    HOST_DEVICE FORCE_INLINE void reset_env(struct DinoState& state, uint32_t seed) {
        state.player_y = 0.0f;
        state.player_vy = 0.0f;
        state.is_ducking = false;
        state.game_speed = 7.0f;
        state.score = 0;
        state.ticks_alive = 0;
        state.rng_state = (seed == 0) ? 123456789 : seed;

        // Change 2: O(1) queue reset instead of looping over slots
        queue_clear(&state.obstacles);
    }

    HOST_DEVICE FORCE_INLINE bool check_collision(const struct DinoState& state) {
        float player_h = state.is_ducking ? PLAYER_H_DUCK : PLAYER_H_STAND;
        float padding = 8.0f;

        // Change 3: Iterate only over active queue count and skip cleared obstacles
        for (int i = 0; i < state.obstacles.count; ++i) {
            const struct Obstacle* obs = queue_get_const(&state.obstacles, i);
            
            if (!obs->is_danger) continue;

            bool overlap_x = (PLAYER_X + padding < obs->x + obs->width - padding) && 
                             (PLAYER_X + PLAYER_W - padding > obs->x + padding);
                            
            bool overlap_y = (state.player_y < obs->y + obs->height - padding) && 
                             (state.player_y + player_h > obs->y + padding);
        
            if (overlap_x && overlap_y) return true;
        }
        return false;
    }

    HOST_DEVICE FORCE_INLINE void step_env(struct DinoState& state, int action, bool& is_game_over) {
        is_game_over = false;
        state.ticks_alive++;
        
        // 1. Process Action
        state.is_ducking = (action == 2 && state.player_y == 0.0f);

        if (action == 1 && state.player_y == 0.0f) {
            state.player_vy = JUMP_VELOCITY;
        }

        // 2. Player Physics
        float grav_multiplier = (action == 2 && state.player_y > 0.0f) ? 2.8f : 1.0f;
        state.player_y += state.player_vy;
        state.player_vy += GRAVITY * grav_multiplier;

        if (state.player_y <= 0.0f) {
            state.player_y = 0.0f;
            state.player_vy = 0.0f;
        }

        // 3. Move Obstacles & Update Threat Status (Change 4)
        for (int i = 0; i < state.obstacles.count; ++i) {
            struct Obstacle* obs = queue_get(&state.obstacles, i);
            obs->x -= state.game_speed;
            
            // Once the back edge passes PLAYER_X, it is no longer a threat
            if (obs->is_danger && (obs->x + obs->width < PLAYER_X)) {
                obs->is_danger = false;
                state.score += 100;
            }
        }

        // Change 5: O(1) Despawn Check on the oldest obstacle (front of queue)
        if (!queue_is_empty(&state.obstacles)) {
            struct Obstacle* oldest = queue_front(&state.obstacles);
            if (oldest->x + oldest->width < 0.0f) {
                queue_pop(&state.obstacles);
            }
        }

        // 4. Speed & Spawning Logic (Change 6: O(1) furthest_x lookup and push)
        if (state.game_speed < MAX_SPEED) {
            state.game_speed += 0.002f;
        }
        
        float min_gap = 200.0f + (state.game_speed * 12.0f);

        if (!queue_is_full(&state.obstacles)) {
            float furthest_x = queue_is_empty(&state.obstacles) 
                               ? 0.0f 
                               : queue_back(&state.obstacles)->x;

            if (furthest_x < 800.0f - min_gap) {
                struct Obstacle new_obs;
                new_obs.x = 800.0f + rand_float(&state.rng_state) * 200.0f;
                new_obs.is_danger = true;
                
                float r = rand_float(&state.rng_state);
                if (r < 0.25f) {
                    // 1. Small ground cactus
                    new_obs.y = 0.0f;
                    new_obs.width = 25.0f;
                    new_obs.height = 45.0f;
                } else if (r < 0.50f) {
                    // 2. Large ground cactus cluster
                    new_obs.y = 0.0f;
                    new_obs.width = 50.0f;
                    new_obs.height = 60.0f;
                } else if (r < 0.75f) {
                    // 3. Walk-under bird (High altitude)
                    new_obs.y = 65.0f;
                    new_obs.width = 45.0f;
                    new_obs.height = 25.0f;
                } else {
                    // 4. Duck-under bird (Mid altitude)
                    new_obs.y = 35.0f;
                    new_obs.width = 45.0f;
                    new_obs.height = 30.0f;
                }

                queue_push(&state.obstacles, new_obs);
            }
        }

        // 5. Terminal State Check
        if (check_collision(state)) {
            is_game_over = true;
        }
    }
}