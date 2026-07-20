#pragma once
#include <string>
#include <vector>
#include "core/Draw.h"
#include "core/helpers.h"
#include "core/Scene.h"
#include "core/Input.h"
#include "assets.h"

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

        Draw::set_y_sort_mode(Draw::YSortMode::YPlusHeight);
    }

    void update(SceneManager& sm, float dt) override {
        float speed = 100.0f * dt;

        if (Input::is_key_pressed(MFB_KB_KEY_LEFT) || Input::is_key_pressed(MFB_KB_KEY_A)) m_playerX -= speed;
        if (Input::is_key_pressed(MFB_KB_KEY_RIGHT) || Input::is_key_pressed(MFB_KB_KEY_D)) m_playerX += speed;
        if (Input::is_key_pressed(MFB_KB_KEY_UP) || Input::is_key_pressed(MFB_KB_KEY_W)) m_playerY -= speed;
        if (Input::is_key_pressed(MFB_KB_KEY_DOWN) || Input::is_key_pressed(MFB_KB_KEY_S)) m_playerY += speed;

        // Add fake score
        if (Input::is_key_just_pressed(MFB_KB_KEY_SPACE)) m_score += 1;

        // Example trigger to go to a Game Over screen if your game had one:
        // if (player_dead) sm.changeScene(std::make_unique<GameOverScene>());
    }

    void draw(std::vector<uint32_t>& buffer) override {
        Draw::text(16, 16, "SCORE:  " + std::to_string(m_score), 0xFFFFFFFF);
        Draw::text(16, 32, "ABCdefUVWxyz1234!\"#$%", 0xFFFFFFFF, 2 /* scale */);
        Draw::text(16, 64, "&\'()*+,-./:;<=>", 0xFFFFFFFF, 3 /* scale */);
        Draw::text(16, 128, "?@[\\]^_`{|}", 0xFFFFFFFF, 4 /* scale */);

        Draw::sprite(
            static_cast<int>(m_playerX),
            static_cast<int>(m_playerY),
            SPRITE_GAME_PAD, // pixels
            SPRITE_GAME_PAD_COMPRESSED_SIZE, // pixels size
            128, // width
            64 // height
        );
    }
};
