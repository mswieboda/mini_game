#pragma once
#include <vector>
#include <string>
#include <variant>
#include <cstdint>
#include "Font.h"
#include "assets/Fonts.h"

struct Transform {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    int z_index = 0;
};

// Renderable variants
struct SpriteRender {
    const uint8_t* pixels;
    uint16_t pixels_size;
    int width;
    int height;
};

// Animated Sprite related concepts, for AnimatedSpriteRender
// Defines a single frame's source rectangle inside the master spritesheet texture
struct SpriteFrame {
    int x;             // Pixel X coordinate on the sheet
    int y;             // Pixel Y coordinate on the sheet
    int width;         // Width of this specific frame
    int height;        // Height of this specific frame
    int duration_ms;   // How long to hold this frame (per-frame custom timing!)
};

// A named sequence of frames (e.g., "idle", "run", "jump")
struct SpriteAnimation {
    std::string name;
    std::vector<int> frame_indices; // Crucial: Reuses/reorders frames (e.g., {0, 1, 2, 1, 0})
    bool loop = true;
};

struct AnimatedSpriteRender {
    const uint8_t* sheet_pixels;       // Pointer to the raw texture file data
    uint16_t sheet_pixels_size;
    int sheet_width;
    int sheet_height;

    // The frame pool (slice your texture once into this vector during setup)
    std::vector<SpriteFrame> master_frames;

    // Playback state
    SpriteAnimation current_anim;      // The current sequence definition
    int current_sequence_index = 0;    // Where we are in the frame_indices loop
    float elapsed_time_ms = 0.0f;      // Ticker that accumulates delta time (dt)
    bool is_playing = true;
};

struct RectangleRender {
    uint32_t color;
    bool fill = true; // TODO: false not draw yet, it will be filled
};

struct TextRender {
    std::string text;
    uint32_t color = 0xFFFFFFFF;
    int scale = 1;
    const FontData* font = &Font::DEFAULT_BLANK;
};

using RenderComponent = std::variant<SpriteRender, AnimatedSpriteRender, RectangleRender, TextRender>;

struct Entity {
    Transform transform;
    RenderComponent visual;
    bool active = true;

    // Simple optional custom tag/ID to identify types (e.g. "player", "coin")
    std::string tag;
};
