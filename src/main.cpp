#include <vector>
#include "Game.h"
#include "core/GameWindow.h"
#include "core/FrameTime.h"
#include "core/Input.h"
#include "core/SceneManager.h"
#include "MiniGameScene.h"

// --- UPDATE --- where game logic updates happens
void frame_updates(GameWindow& window, FrameTime& frame_time, SceneManager& scene_manager) {
    frame_time.update();

    while (frame_time.tick()) {
        if (window.is_active()) {
            // Early out on Escape key if allowed
            if (Game::QUIT_ON_ESC && Input::is_key_pressed(MFB_KB_KEY_ESCAPE)) {
                window.close();
                break;
            }

            // Actual game logic updates
            scene_manager.update(frame_time.fixed_delta());
        } else {
            // Safe stall if window loses focus
            Input::force_clear_all_inputs();
        }

        Input::update_input_state();
        frame_time.consume_step();
    }
}

// --- DRAW --- where drawing happens
void draw(GameWindow& window, FrameTime& frame_time, SceneManager& scene_manager, std::vector<uint32_t>& pixel_buffer) {
    float alpha = frame_time.get_alpha();

    // TODO: we'll use alpha in the draw methods later
    // Use alpha to smoothly slide/interpolate visual coordinates
    // to exactly where they should be at this exact microsecond
    // scene_manager.draw(pixel_buffer, alpha);
    scene_manager.draw(pixel_buffer);

    // Sort, draw everything, and clear the queue cache automatically
    Draw::flush_pipeline(pixel_buffer, 0x00131313);

    window.present(pixel_buffer);
}

// --- MAIN --- init window, frame timing management, pixel buffer, scene manager
// game loop - poll events, updates, draw
int main() {
    GameWindow game_window(Game::TITLE.data(), Game::WIDTH, Game::HEIGHT);
    FrameTime frame_time(Game::TARGET_FPS);

    // Setup input routing
    mfb_set_keyboard_callback(game_window.raw(), Input::keyboard_callback);
    mfb_set_active_callback(game_window.raw(), Input::window_active_callback);

    // Pixel buffer for drawing
    std::vector<uint32_t> pixel_buffer(Game::WIDTH * Game::HEIGHT, 0x00000000);

    SceneManager scene_manager;

    // Initialize and change to the first scene
    scene_manager.changeScene(std::make_unique<MiniGameScene>());

    while (game_window.is_running()) {
        game_window.poll_events();

        frame_updates(game_window, frame_time, scene_manager);

        if (game_window.is_running()) {
            draw(game_window, frame_time, scene_manager, pixel_buffer);
        }
    }

    return 0;
}
