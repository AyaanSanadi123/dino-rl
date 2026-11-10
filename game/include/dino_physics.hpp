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
        
    }

}