#include <vector>
#include "Game.h"
#include "core/GameWindow.h"
#include "core/FrameTime.h"
#include "core/Input.h"
#include "core/SceneManager.h"
#include "MiniGameScene.h"

int main() {
    // Initialize window and frame time management
    GameWindow game_window(Game::TITLE.data(), Game::WIDTH, Game::HEIGHT);
    FrameTime frame_time(Game::TARGET_FPS);

    // Setup input routing
    mfb_set_keyboard_callback(game_window.raw(), Input::keyboard_callback);

    // Pixel buffer for drawing
    std::vector<uint32_t> pixel_buffer(Game::WIDTH * Game::HEIGHT, 0x00000000);

    SceneManager sceneManager;

    // Initialize and change to the first scene
    sceneManager.changeScene(std::make_unique<MiniGameScene>());

    while (game_window.is_running()) {
        game_window.poll_events();

        // Track if a logic frame actually executed this loop iteration
        bool simulated_this_frame = false;

        // High-Precision Fixed Logic Loop
        while (frame_time.tick()) {
            // NOTE: remove this in a true released game so ESC doesn't quit so easily
            if (Game::QUIT_ON_ESC && Input::is_key_pressed(MFB_KB_KEY_ESCAPE)) {
                game_window.close();
                break;
            }

            // scene update --- this is where your actual game logic happens
            sceneManager.update(frame_time.fixed_delta());

            Input::update_input_state();

            frame_time.consume_step();
            simulated_this_frame = true;
        }

        if (simulated_this_frame && game_window.is_running()) {
            // draw (clearing pixels first at beginning of the scene)
            sceneManager.draw(pixel_buffer);

            game_window.present(pixel_buffer);
        }
    }

    return 0;
}
