#pragma once
#include <string>
#include "core/helpers.h"

class MiniGameScene : public Scene {
private:
    float m_playerX;
    float m_playerY;
    int m_score;

public:
    void init(SceneManager& sm) override {
        m_playerX = Game::WIDTH / 2 - 128 / 2;
        m_playerY = Game::HEIGHT - 64 - 16;
        m_score = 0;
    }

    void update(struct mfb_window* window, SceneManager& sm, float dt) override {
        float speed = 100.0f * dt;
        const uint8_t* keys = mfb_get_key_buffer(window);
        
        if (keys[MFB_KB_KEY_LEFT]  || keys[MFB_KB_KEY_A]) m_playerX -= speed;
        if (keys[MFB_KB_KEY_RIGHT] || keys[MFB_KB_KEY_D]) m_playerX += speed;
        if (keys[MFB_KB_KEY_UP]    || keys[MFB_KB_KEY_W]) m_playerY -= speed;
        if (keys[MFB_KB_KEY_DOWN]  || keys[MFB_KB_KEY_S]) m_playerY += speed;

        // Example trigger to go to a Game Over screen if your game had one:
        // if (player_dead) sm.changeScene(std::make_unique<GameOverScene>());
    }

    void draw(std::vector<uint32_t>& buffer) override {
        clear_screen(buffer, 0xFF18181A);
        draw_text(buffer, 20, 10, "SCORE  " + std::to_string(m_score), 0xFFF1FAEE);
        
        draw_sprite_rle(
            buffer,
            static_cast<int>(m_playerX),
            static_cast<int>(m_playerY),
            128,
            64,
            SPRITE_GAME_PAD,
            SPRITE_GAME_PAD_COMPRESSED_SIZE
        );
    }
};
