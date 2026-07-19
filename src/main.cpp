#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <MiniFB.h>

// Bring in the dynamically generated assets (GLOBAL_PALETTE, SPRITE_GAME_PAD, etc.)
#include "assets.h"

// Game Canvas Resolutions
constexpr int WIDTH = 320;
constexpr int HEIGHT = 240;

// Simple 8x8 Bitmap Font Data for alphanumeric renderings (Characters: Space, A-Z)
// 1 byte per row. 1 bits represent fill color, 0 bits represent empty transparency.
const uint8_t MINI_FONT[27][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Space
    {0x18, 0x24, 0x42, 0x42, 0x7E, 0x42, 0x42, 0x42}, // A
    {0x7C, 0x22, 0x22, 0x3C, 0x22, 0x22, 0x22, 0x7C}, // B
    {0x3C, 0x42, 0x40, 0x40, 0x40, 0x40, 0x42, 0x3C}, // C
    {0x78, 0x24, 0x22, 0x22, 0x22, 0x22, 0x24, 0x78}, // D
    {0x7E, 0x40, 0x40, 0x78, 0x40, 0x40, 0x40, 0x7E}, // E
    {0x7E, 0x40, 0x40, 0x78, 0x40, 0x40, 0x40, 0x40}, // F
    {0x3C, 0x42, 0x40, 0x40, 0x4E, 0x42, 0x42, 0x3C}, // G
    {0x42, 0x42, 0x42, 0x7E, 0x42, 0x42, 0x42, 0x42}, // H
    {0x3E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x3E}, // I
    {0x03, 0x01, 0x01, 0x01, 0x01, 0x41, 0x41, 0x3E}, // J
    {0x42, 0x44, 0x48, 0x70, 0x48, 0x44, 0x42, 0x42}, // K
    {0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7E}, // L
    {0x42, 0x66, 0x5A, 0x42, 0x42, 0x42, 0x42, 0x42}, // M
    {0x42, 0x62, 0x52, 0x4A, 0x46, 0x42, 0x42, 0x42}, // N
    {0x3C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C}, // O
    {0x7C, 0x42, 0x42, 0x7C, 0x40, 0x40, 0x40, 0x40}, // P
    {0x3C, 0x42, 0x42, 0x42, 0x42, 0x4A, 0x44, 0x3A}, // Q
    {0x7C, 0x42, 0x42, 0x7C, 0x48, 0x44, 0x42, 0x42}, // R
    {0x3C, 0x42, 0x40, 0x3C, 0x02, 0x02, 0x42, 0x3C}, // S
    {0x7E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08}, // T
    {0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C}, // U
    {0x42, 0x42, 0x42, 0x42, 0x42, 0x24, 0x24, 0x18}, // V
    {0x42, 0x42, 0x42, 0x42, 0x4A, 0x5A, 0x66, 0x42}, // W
    {0x42, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x42}, // X
    {0x42, 0x42, 0x24, 0x18, 0x08, 0x08, 0x08, 0x08}, // Y
    {0x7E, 0x02, 0x04, 0x08, 0x16, 0x20, 0x40, 0x7E}  // Z
};

// --- RENDER HELPERS ---

// Clears the screen buffer with a solid hexadecimal ARGB color
void clear_screen(std::vector<uint32_t>& buffer, uint32_t argb_color) {
    std::fill(buffer.begin(), buffer.end(), argb_color);
}

// 1. Draw a simple solid colored rectangle
void draw_rectangle(std::vector<uint32_t>& buffer, int rx, int ry, int rw, int rh, uint32_t color) {
    int start_x = std::max(0, rx);
    int end_x   = std::min(WIDTH, rx + rw);
    int start_y = std::max(0, ry);
    int end_y   = std::min(HEIGHT, ry + rh);

    for (int y = start_y; y < end_y; ++y) {
        for (int x = start_x; x < end_x; ++x) {
            buffer[y * WIDTH + x] = color;
        }
    }
}

// 2. Draw Text strings onto the frame buffer using our custom bit-array font layout
void draw_text(std::vector<uint32_t>& buffer, int tx, int ty, const std::string& msg, uint32_t color) {
    int cursor_x = tx;
    for (char c : msg) {
        int idx = 0; // Default to space
        if (c >= 'A' && c <= 'Z') idx = (c - 'A') + 1;
        else if (c >= 'a' && c <= 'z') idx = (c - 'a') + 1; // Auto uppercase conversion

        for (int row = 0; row < 8; ++row) {
            uint8_t font_row = MINI_FONT[idx][row];
            int current_y = ty + row;
            if (current_y < 0 || current_y >= HEIGHT) continue;

            for (int col = 0; col < 8; ++col) {
                int current_x = cursor_x + col;
                if (current_x < 0 || current_x >= WIDTH) continue;

                // Check the active state bit left-to-right
                if ((font_row >> (7 - col)) & 0x1) {
                    buffer[current_y * WIDTH + current_x] = color;
                }
            }
        }
        cursor_x += 8; // Advance character spacing offset
    }
}

// 3. Draw an Image: Inflate and render the RLE indexed data stream from our asset pipeline
// Assumes the name extracted is GAME_PAD (Change prefix macro if your file is named differently)
void draw_sprite_rle(std::vector<uint32_t>& buffer, int sx, int sy, int sprite_w, int sprite_h,
                     const uint8_t* rle_data, uint16_t compressed_size) {
    int local_pixel_index = 0;
    uint16_t rle_cursor = 0;

    while (rle_cursor < compressed_size) {
        uint8_t run_length = rle_data[rle_cursor++];
        uint8_t palette_idx = rle_data[rle_cursor++];

        for (uint8_t i = 0; i < run_length; ++i) {
            int local_x = local_pixel_index % sprite_w;
            int local_y = local_pixel_index / sprite_w;
            local_pixel_index++;

            int target_x = sx + local_x;
            int target_y = sy + local_y;

            // Handle standard screen boundaries safely
            if (target_x >= 0 && target_x < WIDTH && target_y >= 0 && target_y < HEIGHT) {
                uint32_t color = GLOBAL_PALETTE[palette_idx];

                // standard alpha skip for fully transparent index mappings (Aseprite index 0 or alpha match)
                if ((color & 0xFF) != 0x00) {
                    buffer[target_y * WIDTH + target_x] = color;
                }
            }
        }
    }
}

int main() {
    // 1. Allocate the software rendering frame buffer
    std::vector<uint32_t> frame_buffer(WIDTH * HEIGHT, 0xFF000000);

    // 2. Initialize the MiniFB display container window wrapper
    struct mfb_window* window = mfb_open_ex("1.44Mb Game Jam Boilerplate", WIDTH, HEIGHT, MFB_WF_RESIZABLE);
    if (!window) {
        std::cerr << "Initialization Error: Could not initialize MiniFB viewport backend.\n";
        return -1;
    }

    // Time Tracking variables for Game Loops
    auto prev_time = std::chrono::high_resolution_clock::now();
    float x_pos = 10.0f; // For rectangle animation demonstration

    std::cout << "Engine execution started successfully. Press ESC to exit viewport loop.\n";

    // 3. Central Application Viewport Handling Engine Loop
    while (true) {
        // Calculate smooth delta time
        auto current_time = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(current_time - prev_time).count();
        prev_time = current_time;

        // Clean Canvas buffer frame at 60 FPS update cycles
        clear_screen(frame_buffer, 0xFF18181A); // Dark charcoal background

        // --- UPDATE & RENDERING DEMONSTRATIONS ---

        // Simple physics update step using independent delta tracking bounds
        x_pos += 45.0f * dt;
        if (x_pos > WIDTH) x_pos = -40.0f;

        // Demonstration 1: Render a Moving Rectangle
        draw_rectangle(frame_buffer, static_cast<int>(x_pos), 40, 40, 25, 0xFFE63946); // Ruby red rectangle

        // Demonstration 2: Render Custom Text Strings
        draw_text(frame_buffer, 20, 90, "HELLO GAME JAM WORKSPACE", 0xFFF1FAEE); // Crisp white text
        draw_text(frame_buffer, 20, 110, "RUNNING AT SIXTY INTERPOLATED FPS", 0x457B9DFF); // Blue descriptor text

        // Demonstration 3: Render an Automated Aseprite Asset Byte Sequence
        // Note: Change 'SPRITE_GAME_PAD' to match the name of your specific file in assets/ uppercase
        // Supply target position, sprite original resolution bounds (adjust if your art isn't 32x32), and arrays
        draw_sprite_rle(frame_buffer, 96, 160, 128, 64, SPRITE_GAME_PAD, SPRITE_GAME_PAD_COMPRESSED_SIZE);

        // 4. Update window context and flush frame buffer arrays directly out onto screen hardware
        mfb_update_state status = mfb_update_ex(window, frame_buffer.data(), WIDTH, HEIGHT);

        // Check if the exit boundary flags are matched or window is closed manually
        if (status < MFB_STATE_OK) {
            window = nullptr;
            break;
        }

        // Keep internal loop operations smooth
        if (!mfb_wait_sync(window)) {
            break;
        }
    }

    std::cout << "Engine loop stopped safely. Context cleanup finished cleanly.\n";
    return 0;
}
