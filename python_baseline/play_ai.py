import sys
import time
import numpy as np
from stable_baselines3 import PPO

try:
    import raylib
except ImportError:
    print("Raylib Python package not found. Install it using: pip import raylib")
    sys.exit(1)

import dino_cpp

def main():
    # 1. Load the trained PPO model
    model_path = "./best_model/best_model.zip"
    print(f"Loading trained model from {model_path}...")
    model = PPO.load(model_path)

    # 2. Initialize Raylib Window
    screen_width = 800
    screen_height = 400
    raylib.InitWindow(screen_width, screen_height, b"C++ Dino Game - AI Agent (PPO)")
    raylib.SetTargetFPS(60)

    # 3. Load Sprite Textures from the assets directory
    dino_run1 = raylib.LoadTexture(b"assets/dino_run1.png")
    dino_run2 = raylib.LoadTexture(b"assets/dino_run2.png")
    dino_duck1 = raylib.LoadTexture(b"assets/dino_duck1.png")
    dino_duck2 = raylib.LoadTexture(b"assets/dino_duck2.png")
    cactus_small = raylib.LoadTexture(b"assets/cactus_small.png")
    cactus_large = raylib.LoadTexture(b"assets/cactus_large.png")
    bird1 = raylib.LoadTexture(b"assets/bird1.png")

    # 4. Initialize C++ game state
    state = dino_cpp.DinoState()
    dino_cpp.reset_env(state, 42)

    print("AI evaluation window opened with full assets! Close window to exit.")

    frame_count = 0

    while not raylib.WindowShouldClose():
        frame_count += 1
        
        # 5. Construct observation vector
        obs_list = [
            state.player_y,
            state.player_vy,
            float(state.is_ducking),
            state.game_speed
        ]
        for obs in state.obstacles:
            obs_list.extend([obs.x, obs.y, obs.width, obs.height, float(obs.active)])
        
        obs_array = np.array(obs_list, dtype=np.float32)

        # 6. Predict action
        action, _states = model.predict(obs_array, deterministic=True)

        # 7. Step C++ physics engine
        reward, done = dino_cpp.step_env(state, int(action))

        if done:
            time.sleep(0.5)
            dino_cpp.reset_env(state, int(time.time()))

        # 8. Render Graphics with Textures
        raylib.BeginDrawing()
        raylib.ClearBackground(raylib.RAYWHITE)

        # Draw Ground Line
        raylib.DrawLine(0, 300, screen_width, 300, raylib.GRAY)

        # Select Dino Texture & Frame Animation (switches every 10 frames)
        anim_toggle = (frame_count // 10) % 2
        if state.is_ducking:
            current_dino = dino_duck2 if anim_toggle == 1 else dino_duck1
        else:
            current_dino = dino_run2 if anim_toggle == 1 else dino_run1

        dino_draw_y = int(300 - state.player_y - 40)
        if current_dino.id > 0:
            raylib.DrawTexture(current_dino, 100, dino_draw_y, raylib.WHITE)
        else:
            raylib.DrawRectangle(100, dino_draw_y, 30, 40, raylib.DARKBLUE)

        # Draw Obstacles (Cacti / Birds based on width/height or properties)
        for obs in state.obstacles:
            if obs.active:
                obs_x = int(obs.x)
                obs_y = int(300 - obs.y - obs.height)
                
                # Choose texture based on obstacle size
                if obs.width > 40 and cactus_large.id > 0:
                    tex = cactus_large
                elif cactus_small.id > 0:
                    tex = cactus_small
                else:
                    tex = None

                if tex and tex.id > 0:
                    raylib.DrawTexture(tex, obs_x, obs_y, raylib.WHITE)
                else:
                    raylib.DrawRectangle(obs_x, obs_y, int(obs.width), int(obs.height), raylib.RED)

        # Draw Telemetry & Score
        score_text = f"Score: {state.score} | Ticks: {state.ticks_alive} | Action: {int(action)}"
        raylib.DrawText(score_text.encode('utf-8'), 20, 20, 20, raylib.DARKGRAY)

        raylib.EndDrawing()

    # 9. Clean up textures
    for tex in [dino_run1, dino_run2, dino_duck1, dino_duck2, cactus_small, cactus_large, bird1]:
        if tex.id > 0:
            raylib.UnloadTexture(tex)

    raylib.CloseWindow()

if __name__ == "__main__":
    main()