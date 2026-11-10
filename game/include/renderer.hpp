#pragma once 

namespace dino {
    class Renderer{
        public:
            // constructor 
            Renderer(int width = 800, int height = 450, const char* title = "Dino CUDA-RL Engine");
            // destructor
            ~Renderer();

            bool should_close() const;
            void draw_test_frame() const;

        private:
            int width_;
            int height_;
    };
}