#include <MiniFB.h>
#include <chrono>
#include <vector>
#include "core/SceneManager.h"
#include "Game.h"
#include "MiniGameScene.h"

int main() {
    std::vector<uint32_t> frame_buffer(Game::WIDTH * Game::HEIGHT, 0xFF000000);
    struct mfb_window* window = mfb_open_ex(
        Game::TITLE.data(),
        Game::WIDTH,
        Game::HEIGHT,
        MFB_WF_RESIZABLE
    );

    if (!window) return -1;

    SceneManager sceneManager;
    // Bootstrap into your first game state cleanly
    sceneManager.changeScene(std::make_unique<MiniGameScene>());

    auto prev_time = std::chrono::high_resolution_clock::now();

    while (true) {
        auto current_time = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(current_time - prev_time).count();
        prev_time = current_time;

        sceneManager.processPendingChanges();
        sceneManager.update(window, dt);
        sceneManager.draw(frame_buffer);

        mfb_update_state status = mfb_update_ex(window, frame_buffer.data(), Game::WIDTH, Game::HEIGHT);
        if (status != MFB_STATE_OK || !mfb_wait_sync(window)) break;
    }
    return 0;
}
