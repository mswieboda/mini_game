#pragma once
#include <string>
#include <vector>
#include "core/Draw.h"
#include "core/helpers.h"
#include "core/Scene.h"
#include "core/Entity.h"
#include "core/Input.h"
#include "assets.h"

class MiniGameScene : public Scene {
private:
    // Cache the entity's index in the `entities` vector, looked up once in init()
    // by tag. Indices survive vector reallocation safely — unlike raw pointers.
    //
    // IMPORTANT — if you ever erase/remove entities at runtime (bullets, enemies, etc.)
    // any cached index pointing AFTER the removed slot becomes wrong.
    // See the comment above entity_index() in Scene.h for the three strategies:
    //   1. Soft-delete  (entity.active = false)  <-- recommended default
    //   2. Swap-and-pop (O(1), unordered)
    //   3. Erase + re-cache (simple, O(n))
    size_t player_idx = SIZE_MAX;
    int score = 0;

    // Private accessor: wraps the index lookup so call sites read like player().transform
    Entity& player() { return entities[player_idx]; }

public:
    void init(SceneManager& sm) override {
        score = 0;
        background_color = 0xFF330033; // slight magenta

        Draw::set_y_sort_mode(Draw::YSortMode::YPlusHeight);

        // Spawn small rect layer (z_index: 0)
        entities.push_back({
            {32, 64, 32, 92, 0}, RectangleRender{0xFF00FF00}, true, "rect"
        });

        // Spawn player (z_index: 10)
        entities.push_back({
            { // Transform
                Game::WIDTH / 2 - 128 / 2, // x
                Game::HEIGHT - 64 - 16, // y
                128, // width
                64, // height
                10 // z-index
            },
            SpriteRender{
                SPRITE_GAME_PAD, // pixels
                SPRITE_GAME_PAD_COMPRESSED_SIZE, // pixels size
                128, // width
                64 // height
            },
            true, // active
            "player" // tag
        });

        // Cache the index once — safe to hold forever since indices survive reallocation
        player_idx = entity_index("player");
    }

    void update(SceneManager& sm, float dt) override {
        float speed = 100.0f * dt;

        if (Input::is_key_pressed(MFB_KB_KEY_LEFT) || Input::is_key_pressed(MFB_KB_KEY_A))
            player().transform.x -= speed;
        if (Input::is_key_pressed(MFB_KB_KEY_RIGHT) || Input::is_key_pressed(MFB_KB_KEY_D))
            player().transform.x += speed;
        if (Input::is_key_pressed(MFB_KB_KEY_UP) || Input::is_key_pressed(MFB_KB_KEY_W))
            player().transform.y -= speed;
        if (Input::is_key_pressed(MFB_KB_KEY_DOWN) || Input::is_key_pressed(MFB_KB_KEY_S))
            player().transform.y += speed;

        // Add fake score
        if (Input::is_key_just_pressed(MFB_KB_KEY_SPACE)) score += 1;

        // Example trigger to go to a Game Over screen if your game had one:
        // if (player_dead) sm.change_scene(std::make_unique<GameOverScene>());
    }

    // NOTE: `draw_entities` called internally in `Scene::draw` drawing all `entities`

    // NOTE: override `draw_custom` to draw non-Entities like lines, rects, or
    // images like icons, backgrounds, whatever you want
};
