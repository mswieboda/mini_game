#pragma once
#include <string>
#include <vector>
#include "core/Draw.h"
#include "core/helpers.h"
#include "core/Scene.h"
#include "core/Entity.h"
#include "core/Input.h"
#include "core/Audio.h"
#include "SFX.h"
#include "assets/ImageData.h"
#include "assets/MusicData.h"

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
    size_t animation_idx = SIZE_MAX;
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
                Assets::Images::game_pad, // pixels
                Assets::Images::game_pad_len, // pixels size
                128, // width
                64 // height
            },
            true, // active
            "player" // tag
        });

        // Cache the index once — safe to hold forever since indices survive reallocation
        player_idx = entity_index("player");

        // Set up animated test with the game-pad asset
        // Define a pool of frame coordinates from your texture sheet
        // Let's pretend your spritesheet has 3 horizontal frames, each 32x32 pixels
        std::vector<SpriteFrame> pad_frames = {
            { 0,  0, 32, 32, 150 }, // Frame 0: holds for 150ms
            { 32, 0, 32, 32, 150 }, // Frame 1: holds for 150ms
            { 64, 0, 32, 32, 150 }  // Frame 2: holds for 150ms
        };

        // 2. Define the playlist/blueprint sequence
        // This plays frames: 0 -> 1 -> 2 -> 1, then loops smoothly back to 0!
        SpriteAnimation walk_anim = {
            "walk", // name
            {0, 1, 2, 1}, // frame_indices sequence (Recycles frame 1!)
            true // loops automatically
        };

        // 3. Assemble and push the Animated entity into the scene vector
        entities.push_back({
            { 100.0f, 100.0f, 32.0f, 32.0f, 10 }, // Transform (x, y, w, h, z)
            AnimatedSpriteRender{
                Assets::Images::game_pad, // sheet_pixels pointer
                Assets::Images::game_pad_len, // data size
                128, 64, // total sheet width, height
                pad_frames, // our master frame pool
                walk_anim // active animation blueprint
            },
            true, // active
            "animation" // tag
        });

        animation_idx = entity_index("animation");
    }

    void update(SceneManager& sm, float dt) override {
        float speed = 100.0f * dt;

        // --- player input ---
        if (Input::is_key_pressed(MFB_KB_KEY_LEFT) || Input::is_key_pressed(MFB_KB_KEY_A))
            player().transform.x -= speed;
        if (Input::is_key_pressed(MFB_KB_KEY_RIGHT) || Input::is_key_pressed(MFB_KB_KEY_D))
            player().transform.x += speed;
        if (Input::is_key_pressed(MFB_KB_KEY_UP) || Input::is_key_pressed(MFB_KB_KEY_W))
            player().transform.y -= speed;
        if (Input::is_key_pressed(MFB_KB_KEY_DOWN) || Input::is_key_pressed(MFB_KB_KEY_S))
            player().transform.y += speed;

        // --- Add fake score ---
        if (Input::is_key_just_pressed(MFB_KB_KEY_SPACE)) score += 1;

        // Example trigger to go to a Game Over screen if your game had one:
        // if (player_dead) sm.change_scene(std::make_unique<GameOverScene>());

        // --- test out animation ---
        if (animation_idx == SIZE_MAX) return;

        // Fetch a safe mutable reference to our animation entity
        Entity& animation = entities[animation_idx];

        // Grab a pointer to the variant data if it matches our animated struct
        if (auto* anim = std::get_if<AnimatedSpriteRender>(&animation.visual)) {
            // --- Interactive Testing via SPACE Key ---
            if (Input::is_key_just_pressed(MFB_KB_KEY_SPACE)) {
                // Toggle between Pause and Play states instantly!
                anim->is_playing = !anim->is_playing;

                Log::fmt("Animation state changed! is_playing = %s\n",
                         anim->is_playing ? "TRUE" : "FALSE");
            }
        }

        // --- audio sfx ---
        // Play presets directly
        if (Input::is_key_just_pressed(MFB_KB_KEY_P))
            Audio::play_sfx(SFX::phaser());
        if (Input::is_key_just_pressed(MFB_KB_KEY_C))
            Audio::play_sfx(SFX::coin());
        if (Input::is_key_just_pressed(MFB_KB_KEY_E))
            Audio::play_sfx(SFX::explosion());

        if (Input::is_key_just_pressed(MFB_KB_KEY_X)) {
            // Or customize sound parameters on the fly
            SfxrParams custom_hit;
            custom_hit.wave_type = SAWTOOTH;
            custom_hit.start_frequency = 0.5f;
            custom_hit.slide = -0.15f;
            custom_hit.sustain_time = 0.25f;
            custom_hit.decay_time = 0.55f;

            Audio::play_sfx(custom_hit);
        }

        // --- music ---
        // Load and start playing in your game setup:
        if (Input::is_key_just_pressed(MFB_KB_KEY_M)) {
            if (!Audio::is_music_loaded()) {
                // Load and start playing for the first time
                if (Audio::load_music_from_memory(Assets::Music::awm, Assets::Music::awm_len)) {
                    Audio::play_music(/*loop=*/ true);
                }
            } else {
                // Toggle pause/resume state
                Audio::toggle_music();
            }
        }
    }

    // NOTE: `draw_entities` called internally in `Scene::draw` drawing all `entities`

    // NOTE: override `draw_custom` to draw non-Entities like lines, rects, or
    // images like icons, backgrounds, whatever you want
};
