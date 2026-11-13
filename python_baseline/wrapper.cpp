#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "dino_physics.hpp"

namespace py = pybind11;

PYBIND11_MODULE(dino_cpp, m) {
    m.doc() = "C++ high-performance physics engine wrapper for Dino RL";

    py::class_<dino::Obstacle>(m, "Obstacle")
        .def(py::init<>())
        .def_readwrite("x", &dino::Obstacle::x)
        .def_readwrite("y", &dino::Obstacle::y)
        .def_readwrite("width", &dino::Obstacle::width)
        .def_readwrite("height", &dino::Obstacle::height)
        .def_readwrite("active", &dino::Obstacle::active);

    py::class_<dino::DinoState>(m, "DinoState")
        .def(py::init<>())
        .def_readwrite("player_y", &dino::DinoState::player_y)
        .def_readwrite("player_vy", &dino::DinoState::player_vy)
        .def_readwrite("is_ducking", &dino::DinoState::is_ducking)
        .def_readwrite("game_speed", &dino::DinoState::game_speed)
        .def_readwrite("score", &dino::DinoState::score)
        .def_readwrite("ticks_alive", &dino::DinoState::ticks_alive)
        // Custom property wrapper for C-style obstacle array
        .def_property("obstacles", 
            [](dino::DinoState &s) {
                return std::vector<dino::Obstacle>(s.obstacles, s.obstacles + 5);
            },
            [](dino::DinoState &s, const std::vector<dino::Obstacle> &vec) {
                for (size_t i = 0; i < std::min(vec.size(), size_t(5)); ++i) {
                    s.obstacles[i] = vec[i];
                }
            }
        );

    m.def("reset_env", &dino::reset_env, "Reset the C++ game state");

    m.def("step_env", [](dino::DinoState& state, int action) {
        float reward = 0.0f;
        bool done = false;
        dino::step_env(state, action, reward, done);
        return std::make_pair(reward, done);
    }, "Step the C++ game forward by 1 frame");
}
