import gymnasium as gymnasium
from gymnasium import spaces
import numpy as np
import dino_cpp  # Imports your compiled C++ module from build/

import sys
import os

# Add the build folder to Python's search path dynamically
build_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '../build'))
sys.path.append(build_dir)

import dino_cpp

class DinoGameEnv(gymnasium.Env):
    metadata = {"render_modes": ["human"]}

    def __init__(self):
        super(DinoGameEnv, self).__init__()
        
        # Action space: 0 = Run, 1 = Jump, 2 = Duck
        self.action_space = spaces.Discrete(3)
        
        # Observation space: 1D vector tracking player and obstacle states
        # [player_y, player_vy, is_ducking, game_speed, obs1_x, obs1_y, obs1_w, obs1_h, obs1_active, ...]
        self.observation_space = spaces.Box(
            low=-np.inf, high=np.inf, shape=(29,), dtype=np.float32
        )
        
        self.state = dino_cpp.DinoState()

    def reset(self, seed=None, options=None):
        super().reset(seed=seed)
        if seed is not None:
            dino_cpp.reset_env(self.state, int(seed))
        else:
            dino_cpp.reset_env(self.state, 42)
            
        return self._get_obs(), {}

    def step(self, action):
        # Step C++ physics forward by 1 frame
        reward, terminated = dino_cpp.step_env(self.state, int(action))
        
        obs = self._get_obs()
        truncated = False  # No time limit truncation, ends only on crash
        
        info = {"score": self.state.score}
        return obs, reward, terminated, truncated, info

    def _get_obs(self):
        obs_list = [
            self.state.player_y,
            self.state.player_vy,
            float(self.state.is_ducking),
            self.state.game_speed
        ]
        # Pack obstacle details into flat vector
        for obs in self.state.obstacles:
            obs_list.extend([obs.x, obs.y, obs.width, obs.height, float(obs.active)])
            
        return np.array(obs_list, dtype=np.float32)