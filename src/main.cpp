#include <MiniFB.h>
#include <vector>
#include "core/Input.h"
#include "core/SceneManager.h"
#include "Game.h"
#include "MiniGameScene.h"

int main() {
    // Pixel buffer for drawing
    std::vector<uint32_t> pixel_buffer(Game::WIDTH * Game::HEIGHT, 0xFF000000);

    // Create the MFB window
    struct mfb_window* window = mfb_open_ex(
        Game::TITLE.data(),
        Game::WIDTH,
        Game::HEIGHT,
        MFB_WF_RESIZABLE
    );

    if (!window) return -1;

    mfb_set_keyboard_callback(window, Input::keyboard_callback);

    SceneManager sceneManager;

    // Initialize the first scene
    sceneManager.changeScene(std::make_unique<MiniGameScene>());

    bool running = true;
    FrameTimer timer;
    FrameLimiter fps_limiter(Game::TARGET_FPS);

    while (running) {
        // set up input
        Input::update_input_state();

        // get events
        mfb_update_state state = mfb_update_events(window);

        if (state < 0) {
            running = false;
            break;
        }

        // NOTE: remove this in a true released game so ESC doesn't quit so easily
        if (Game::QUIT_ON_ESC && Input::is_key_pressed(MFB_KB_KEY_ESCAPE)) {
            running = false;
            break;
        }

        // main scene manager update and draw
        float dt = timer.delta();

        // update
        sceneManager.update(window, dt);

        // draw
        sceneManager.draw(pixel_buffer);

        // persist draw, "present" in most engines
        state = mfb_update_ex(window, pixel_buffer.data(), Game::WIDTH, Game::HEIGHT);

        if (state < 0) {
            running = false;
        }

        // FPS frame limiter
        // if (running) {
        //     fps_limiter.wait(dt);
        // }
    }

    return 0;
}
