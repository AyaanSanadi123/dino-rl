#include "renderer.hpp"


int main(){
    dino::Renderer renderer(800, 450, "Dino CUDA-RL - Milestone 1 Test");

    while (!renderer.should_close()) {
        renderer.draw_test_frame();
    }
    return 0;
}