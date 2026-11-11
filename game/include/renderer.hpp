#pragma once
#include <raylib.h>
#include "dino_physics.hpp"

namespace dino {

class Renderer {
public:
    Renderer(int width, int height, const char* title);
    ~Renderer();

    bool should_close() const;
    void draw_state(const DinoState& state, bool is_game_over) const;

private:
    int width_;
    int height_;
    
    // Texture assets
    Texture2D tex_dino_run1;
    Texture2D tex_dino_run2;
    Texture2D tex_dino_duck1;
    Texture2D tex_dino_duck2;
    Texture2D tex_cactus_small;
    Texture2D tex_cactus_large;
    Texture2D tex_bird1;
    Texture2D tex_bird2;
};

} // namespace dino