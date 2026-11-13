import os
from stable_baselines3 import PPO
from stable_baselines3.common.env_checker import check_env
from dino_env import DinoGameEnv

# 1. Initialize environment and audit compliance
env = DinoGameEnv()
check_env(env)

# 2. Setup logging directory
log_dir = "./dino_tensorboard/"
os.makedirs(log_dir, exist_ok=True)

# 3. Configure PPO model with exploration entropy coefficient
model = PPO(
    policy="MlpPolicy",
    env=env,
    learning_rate=3.0e-4,
    n_steps=2048,
    batch_size=64,
    n_epochs=10,
    gamma=0.99,
    ent_coef=0.01,  # Essential to encourage jumping/ducking exploration
    verbose=1,
    tensorboard_log=log_dir
)

print("Starting PPO training on C++ Dino Engine...")
model.learn(total_timesteps=300_000)

# 4. Save trained weights
model.save("dino_ppo_baseline")
print("Training complete! Model saved as dino_ppo_baseline.zip")