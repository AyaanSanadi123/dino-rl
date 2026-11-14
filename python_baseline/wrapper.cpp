// python_baseline/wrapper.cpp
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <algorithm>
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
        .def_readwrite("is_danger", &dino::Obstacle::is_danger);

    py::class_<dino::DinoState>(m, "DinoState")
        .def(py::init<>())
        .def_readwrite("player_y", &dino::DinoState::player_y)
        .def_readwrite("player_vy", &dino::DinoState::player_vy)
        .def_readwrite("is_ducking", &dino::DinoState::is_ducking)
        .def_readwrite("game_speed", &dino::DinoState::game_speed)
        .def_readwrite("score", &dino::DinoState::score)
        .def_readwrite("ticks_alive", &dino::DinoState::ticks_alive)
        // Unpacks the ring buffer in strict FIFO order (closest to furthest)
        .def_property("obstacles", 
            [](const dino::DinoState &s) {
                std::vector<dino::Obstacle> active_obs;
                active_obs.reserve(s.obstacles.count);
                for (int i = 0; i < s.obstacles.count; ++i) {
                    const dino::Obstacle* obs = dino::queue_get_const(&s.obstacles, i);
                    active_obs.push_back(*obs);
                }
                return active_obs;
            },
            [](dino::DinoState &s, const std::vector<dino::Obstacle> &vec) {
                dino::queue_clear(&s.obstacles);
                size_t count = std::min(vec.size(), static_cast<size_t>(MAX_OBSTACLES));
                for (size_t i = 0; i < count; ++i) {
                    dino::queue_push(&s.obstacles, vec[i]);
                }
            }
        );

    m.def("reset_env", &dino::reset_env, "Reset the C++ game state");

    m.def("step_env", [](dino::DinoState& state, int action) {
        bool done = false;
        dino::step_env(state, action, done);
        return done;
    }, "Step the C++ game forward by 1 frame and return whether the episode terminated");
}