#pragma once
#include <vector>
#include <string>
#include <variant>
#include <cstdint>

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

struct RectangleRender {
    uint32_t color;
    bool fill = true; // TODO: false not draw yet, it will be filled
};

struct TextRender {
    std::string text;
    uint32_t color;
    int scale = 1;
};

using RenderComponent = std::variant<SpriteRender, RectangleRender, TextRender>;

struct Entity {
    Transform transform;
    RenderComponent visual;
    bool active = true;
    
    // Simple optional custom tag/ID to identify types (e.g. "player", "coin")
    std::string tag; 
};
