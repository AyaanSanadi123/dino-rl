import numpy as np
import torch
from stable_baselines3 import PPO
from dino_env import DinoGameEnv

def main():
    # 1. Load your trained model
    model_path = "dino_ppo_baseline.zip"
    print(f"Loading trained model from {model_path}...")
    model = PPO.load(model_path)

    # 2. Inspect Neural Network Architecture (MLP Policy)
    print("\n=== POLICY NETWORK ARCHITECTURE ===")
    print(model.policy)

    # 3. Inspect Model Weights & Biases
    print("\n=== WEIGHTS & BIASES INSPECTION ===")
    policy_state_dict = model.policy.state_dict()
    for name, param in policy_state_dict.items():
        print(f"Layer: {name} | Shape: {param.shape}")

    # 4. Test a live inference prediction on a custom environment state
    env = DinoGameEnv()
    obs, _ = env.reset()
    
    # Ask the trained model what action to take given the current observation vector
    action, _states = model.predict(obs, deterministic=True)
    
    print("\n=== LIVE INFERENCE TEST ===")
    print(f"Observation Vector: {obs}")
    print(f"Chosen Action by PPO Agent: {action} (0=Run, 1=Jump, 2=Duck)")

if __name__ == "__main__":
    main()