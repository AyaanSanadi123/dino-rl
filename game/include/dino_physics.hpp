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
    constexpr int MAX_OBSTACLES = 3;

    // define a obstical 
    struct Obstacle {
        float x;
        float y;
        float width;
        float height;
        bool active;
    }
    struct DinoState{
        float player_y; // this is the vertical position of the dinosour
        float player_vy; // this is the vertical velocity of the dino, reduces with time as gravity acts 
        bool is_ducking;

        float game_speed;
        int score;
        int ticks_alive; // this is the amount of frames the player has been alive, use to calculate score 

        uint32_t rng_state; // this is a random number generator, that works on both cpu and gpu indifferently 
        Obstacle Obstacles[MAX_OBSTACLES];
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
    
        for (int i = 0; i < MAX_OBSTACLES; ++i) {
            if (!state.obstacles[i].active) continue;
            const auto &obs = state.obstacles[i];

            bool overlap_x = (PLAYER_X < obs.x + obs.width) && (PLAYER_X + PLAYER_W > obs.x);
            bool overlap_y = (state.player_y < obs.y + obs.height) && (state.player_y + player_h > obs.y);
        
            if (overlap_x && overlap_y) return true;
        }
        return false;
    }
    // things that need to happen after every frame passes 
    HOST_DEVICE FORCE_INLINE void step_env(DinoState& state, int action, float& reward, bool& done){
        // before state -> DinoState& state
        // the decision made for this specific frame -> action (0 -> RUN,1 -> JUMP,2 ->DUCK)
        // this function applies logic for a single tick 
        // it checks the applied action , applies gravity if required 
        // moves the objects to the left 
        // renders new obsticales if required 

        // this reward is not part of the game engine, rather it is for RL purposes 
        // the done is the reset switch, only becomes true if the dinosour collides with the obstical

        // this is setting the context for the current frame 
        reward = 0.05f; // Base survival reward
        done = false;
        state.ticks_alive++;
        
        // process the action 
        state.is_ducking = (action == 2 && state.player_y == 0.0f); // if the player is ducking, then action must be 2 and vertical elevation must be 0

        if(action == 1 && state.player_y == 0.0f) state.player_vy = JUMP_VELOCITY;

        // 2. Player Physics
        // this checks, if the player is airborn and is trying to duck, if so, gravity is set to 2.8, else 1
        float grav_multiplier = (action == 2 && state.player_y > 0.0f) ? 2.8f : 1.0f;
        state.player_y += state.player_vy; // move the dino, upwards wrt its current velocity
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
            // heuristic for the min gap required, else the dino will jump onto the next cactus
            float min_gap = 250.0f + (state.game_speed * 15.0f);

            if (furthest_x < 800.0f - min_gap) {
            // Find inactive slot
            for (int i = 0; i < MAX_OBSTACLES; ++i) {
                if (!state.obstacles[i].active) {
                    state.obstacles[i].active = true;
                    state.obstacles[i].x = 800.0f + rand_float(&state.rng_state) * 200.0f;
                    
                    float r = rand_float(&state.rng_state);
                    if (r < 0.4f) {
                        // Small ground cactus
                        state.obstacles[i].y = 0.0f;
                        state.obstacles[i].width = 25.0f;
                        state.obstacles[i].height = 45.0f;
                    } else if (r < 0.7f) {
                        // Large ground cactus cluster
                        state.obstacles[i].y = 0.0f;
                        state.obstacles[i].width = 50.0f;
                        state.obstacles[i].height = 60.0f;
                    } else {
                        // Pterodactyl (Requires ducking)
                        state.obstacles[i].y = 42.0f;
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