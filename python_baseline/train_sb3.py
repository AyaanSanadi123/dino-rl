import os
from stable_baselines3 import PPO
from stable_baselines3.common.callbacks import EvalCallback, StopTrainingOnNoModelImprovement
from stable_baselines3.common.monitor import Monitor
from dino_env import DinoGameEnv

def main():
    # 1. Setup directories for saving checkpoints and best weights
    log_dir = "./logs/"
    best_model_dir = "./best_model/"
    os.makedirs(log_dir, exist_ok=True)
    os.makedirs(best_model_dir, exist_ok=True)

    # 2. Initialize single-threaded environments wrapped with Monitor
    env = Monitor(DinoGameEnv())
    eval_env = Monitor(DinoGameEnv())

    # 3. Setup Early Stopping & Evaluation Callbacks
    # - Stops training if no new high score is hit in 15 consecutive evaluation checks
    stop_callback = StopTrainingOnNoModelImprovement(
        max_no_improvement_evals=15, 
        min_evals=20
    )

    eval_callback = EvalCallback(
        eval_env=eval_env,
        best_model_save_path=best_model_dir,
        log_path=log_dir,
        eval_freq=2048,           # Evaluates once every rollout epoch (2048 steps)
        n_eval_episodes=5,
        deterministic=True,
        render=False,
        callback_after_eval=stop_callback
    )

    # 4. Initialize PPO configured for your RTX 4050 GPU
    model = PPO(
        policy="MlpPolicy",
        env=env,
        device="cuda",            # <--- Forces PyTorch to run on your RTX 4050 GPU
        learning_rate=3.0e-4,
        n_steps=2048,             # Rollout buffer size
        batch_size=256,           # Increased batch size to better feed the GPU parallel cores
        n_epochs=10,              # Optimization epochs per rollout
        gamma=0.99,
        ent_coef=0.01,            # Keeps exploration active
        verbose=1
    )

    # 5. Target iterations & total timesteps
    target_iterations = 100000
    target_timesteps = target_iterations * 2048

    print("--- Starting GPU-Accelerated Phase-2 Training ---")
    print(f"Target: {target_iterations:,} iterations ({target_timesteps:,} total timesteps) on RTX 4050.")
    print("Early stopping enabled: Will halt if no improvement is seen for 15 evaluations.")
    
    model.learn(
        total_timesteps=target_timesteps,
        callback=eval_callback
    )

    print(f"\nTraining completed or early-stopped.")
    print(f"Your best performing model is securely saved at: {best_model_dir}best_model.zip")

if __name__ == "__main__":
    main()