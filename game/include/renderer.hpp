// game/include/renderer.hpp
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
};

} // namespace dino