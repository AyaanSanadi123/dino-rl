import time
from stable_baselines3 import PPO
from dino_env import DinoGameEnv

def main():
    # 1. Initialize the C++ Gymnasium environment
    env = DinoGameEnv()

    # 2. Load the trained PPO model from your best checkpoints
    model_path = "./best_model/best_model.zip"
    print(f"Loading trained model from {model_path}...")
    model = PPO.load(model_path)

    # 3. Run evaluation episodes
    num_episodes = 5
    print(f"\nRunning evaluation for {num_episodes} episodes...")

    for ep in range(num_episodes):
        obs, _ = env.reset()
        done = False
        total_reward = 0.0
        steps = 0

        while not done:
            # Predict the optimal action using the trained policy (deterministic=True for best play)
            action, _states = model.predict(obs, deterministic=True)
            
            # Step the C++ environment forward
            obs, reward, terminated, truncated, info = env.step(int(action))
            done = terminated or truncated
            
            total_reward += reward
            steps += 1

        score = info.get("score", 0)
        print(f"Episode {ep + 1}: Score = {score} | Ticks Alive = {steps} | Total Reward = {total_reward:.2f}")

    print("\nEvaluation complete!")

if __name__ == "__main__":
    main()