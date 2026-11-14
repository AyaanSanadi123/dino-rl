import os
import sys
import time
import numpy as np
from stable_baselines3 import PPO

try:
    import raylib
except ImportError:
    print("Raylib Python package not found. Install it using: pip install raylib")
    sys.exit(1)

import dino_cpp

def main():
    # 1. Load the trained PPO model
    model_path = "./best_model/best_model.zip"
    print(f"Loading trained model from {model_path}...")
    model = PPO.load(model_path)

    # 2. Initialize Raylib Window (800x400 matching renderer.hpp dimensions)
    screen_width = 800
    screen_height = 400
    raylib.InitWindow(screen_width, screen_height, b"C++ Dino Game - AI Agent (PPO)")
    raylib.SetTargetFPS(60)

    # 3. Load All Sprite Textures using absolute paths safely
    base_dir = os.path.dirname(os.path.abspath(__file__))
    assets_dir = os.path.join(base_dir, "assets")

    def get_asset_path(filename):
        return os.path.join(assets_dir, filename).encode('utf-8')

    dino_run1 = raylib.LoadTexture(get_asset_path("dino_run1.png"))
    dino_run2 = raylib.LoadTexture(get_asset_path("dino_run2.png"))
    dino_duck1 = raylib.LoadTexture(get_asset_path("dino_duck1.png"))
    dino_duck2 = raylib.LoadTexture(get_asset_path("dino_duck2.png"))
    cactus_small = raylib.LoadTexture(get_asset_path("cactus_small.png"))
    cactus_large = raylib.LoadTexture(get_asset_path("cactus_large.png"))
    bird1 = raylib.LoadTexture(get_asset_path("bird1.png"))
    bird2 = raylib.LoadTexture(get_asset_path("bird2.png"))

    # 4. Initialize C++ game state
    state = dino_cpp.DinoState()
    dino_cpp.reset_env(state, 42)

    print("AI evaluation window opened with full assets! Close window to exit.")

    while not raylib.WindowShouldClose():
        # 5. Construct normalized 24-float observation vector matching DinoGameEnv
        obs_list = [
            state.player_y / 150.0,
            state.player_vy / 15.0,
            1.0 if state.is_ducking else 0.0,
            (state.game_speed - 7.0) / (16.0 - 7.0)
        ]

        # Filter active threats from the ring buffer using is_danger
        threats = [o for o in state.obstacles if o.is_danger]

        # Pack 5 obstacle slots (20 floats)
        for i in range(5):
            if i < len(threats):
                o = threats[i]
                obs_list.extend([
                    (o.x - 80.0) / 800.0,
                    o.y / 100.0,
                    o.width / 60.0,
                    o.height / 60.0
                ])
            else:
                obs_list.extend([1.5, 0.0, 0.0, 0.0])
        
        obs_array = np.array(obs_list, dtype=np.float32)

        # 6. Predict action from trained PPO model
        action, _states = model.predict(obs_array, deterministic=True)

        # 7. Step C++ physics engine
        terminated = dino_cpp.step_env(state, int(action))

        if terminated:
            time.sleep(0.5)
            dino_cpp.reset_env(state, int(time.time()))

        # 8. Render Graphics (Mirrors renderer.cpp layout)
        raylib.BeginDrawing()
        raylib.ClearBackground(raylib.RAYWHITE)

        # Ground line mapped to window proportion (renderer uses ground_y = 380.0f for 400 height)
        ground_y = 380.0
        raylib.DrawLine(0, int(ground_y), screen_width, int(ground_y), raylib.BLACK)

        # Animation frame calculation (swaps every 5 ticks, matching renderer.cpp)
        anim_frame = int((state.ticks_alive / 5) % 2)

        # Draw the Player with exact physics hitbox sizing
        player_h = 30.0 if state.is_ducking else 60.5  # PLAYER_H_DUCK vs PLAYER_H_STAND
        player_w = 44.0                                # PLAYER_X / PLAYER_W standard
        player_x_pos = 80.0                            # PLAYER_X
        screen_player_y = ground_y - state.player_y - player_h

        if state.player_y > 0.0:
            current_dino = dino_run1
        elif state.is_ducking:
            current_dino = dino_duck1 if anim_frame == 0 else dino_duck2
        else:
            current_dino = dino_run1 if anim_frame == 0 else dino_run2

        if current_dino.id > 0:
            # Pass tuples instead of Raylib structs for compatibility with Python bindings
            source_rect = (0.0, 0.0, float(current_dino.width), float(current_dino.height))
            dest_rect = (player_x_pos, screen_player_y, player_w, player_h)
            origin = (0.0, 0.0)
            raylib.DrawTexturePro(current_dino, source_rect, dest_rect, origin, 0.0, raylib.WHITE)
        else:
            raylib.DrawRectangle(int(player_x_pos), int(screen_player_y), int(player_w), int(player_h), raylib.DARKBLUE)

        # Draw Obstacles from state.obstacles ring buffer
        for obs in state.obstacles:
            screen_obs_y = ground_y - obs.y - obs.height
            
            if obs.y > 0.0: # Pterodactyl
                obs_tex = bird1 if anim_frame == 0 else bird2
            elif obs.width > 30.0:
                obs_tex = cactus_large
            else:
                obs_tex = cactus_small

            if obs_tex and obs_tex.id > 0:
                obs_source = (0.0, 0.0, float(obs_tex.width), float(obs_tex.height))
                obs_dest = (obs.x, screen_obs_y, obs.width, obs.height)
                raylib.DrawTexturePro(obs_tex, obs_source, obs_dest, (0.0, 0.0), 0.0, raylib.WHITE)
            else:
                raylib.DrawRectangle(int(obs.x), int(screen_obs_y), int(obs.width), int(obs.height), raylib.RED)

        # 9. UI Elements & Telemetry
        score_text = f"SCORE: {state.score} | SPEED: {state.game_speed:.2f} | ACTION: {int(action)}"
        raylib.DrawText(score_text.encode('utf-8'), 20, 20, 20, raylib.DARKGRAY)

        raylib.EndDrawing()

    # 10. Clean up GPU VRAM textures on exit
    for tex in [dino_run1, dino_run2, dino_duck1, dino_duck2, cactus_small, cactus_large, bird1, bird2]:
        if tex.id > 0:
            raylib.UnloadTexture(tex)

    raylib.CloseWindow()

if __name__ == "__main__":
    main()