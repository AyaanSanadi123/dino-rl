import gymnasium as gym
from gymnasium import spaces
import numpy as np
import dino_cpp

PLAYER_X = 80.0
MAX_OBSTACLES = 5

class DinoGameEnv(gym.Env):
    """Gymnasium RL wrapper around the pure C/C++ Dino physics engine."""

    def __init__(self):
        super().__init__()
        self.state = dino_cpp.DinoState()

        # Actions: 0 = Run/Stand, 1 = Jump, 2 = Duck/Fast-Fall
        self.action_space = spaces.Discrete(3)

        # Observation: 4 player features + 5 obstacles * 4 features (dx, y, w, h) = 24 floats
        self.observation_space = spaces.Box(
            low=-2.0,
            high=2.0,
            shape=(24,),
            dtype=np.float32
        )

        self.rng = np.random.default_rng()

    def reset(self, seed=None, options=None):
        super().reset(seed=seed)
        if seed is not None:
            self.rng = np.random.default_rng(seed)

        cpp_seed = int(self.rng.integers(1, 2**31 - 1))
        dino_cpp.reset_env(self.state, cpp_seed)

        return self._get_obs(), {}

    def step(self, action):
        # 1. Snapshot pre-step state to detect transitions
        prev_score = self.state.score
        was_grounded = (self.state.player_y == 0.0)

        # 2. Advance pure physics engine by 1 frame
        terminated = dino_cpp.step_env(self.state, int(action))

        # 3. Compute standardized RL reward & punishment
        if terminated:
            reward = -1.0
        else:
            reward = 0.01  # Base survival reward per frame

            # Obstacle cleared bonus (score increments when is_danger flips to False)
            if self.state.score > prev_score:
                reward += 1.0

            # Small action tax for initiating a jump to prevent blind jump-spamming
            if int(action) == 1 and was_grounded:
                reward -= 0.02

        obs = self._get_obs()
        truncated = False
        info = {
            "score": self.state.score,
            "ticks_alive": self.state.ticks_alive
        }

        return obs, float(reward), bool(terminated), truncated, info

    def _get_obs(self):
        # 1. Normalized Player State (4 floats)
        obs = [
            self.state.player_y / 150.0,                  # Altitude [0, ~1.0]
            self.state.player_vy / 15.0,                  # Vertical velocity [-1.0, 1.0]
            1.0 if self.state.is_ducking else 0.0,        # Ducking flag {0.0, 1.0}
            (self.state.game_speed - 7.0) / (16.0 - 7.0)  # Normalized speed [0.0, 1.0]
        ]

        # 2. Extract active threats in FIFO order from the C ring buffer
        threats = [o for o in self.state.obstacles if o.is_danger]

        # 3. Pack 5 obstacle slots (20 floats)
        for i in range(MAX_OBSTACLES):
            if i < len(threats):
                o = threats[i]
                obs.extend([
                    (o.x - PLAYER_X) / 800.0,  # Relative distance to front edge
                    o.y / 100.0,               # Elevation (0.0=cactus, 0.35=mid bird, 0.65=high bird)
                    o.width / 60.0,            # Normalized width
                    o.height / 60.0            # Normalized height
                ])
            else:
                # Empty slot padding (far away, zero dimensions)
                obs.extend([1.5, 0.0, 0.0, 0.0])

        return np.array(obs, dtype=np.float32)