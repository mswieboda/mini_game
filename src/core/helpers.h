#pragma once
#include <cstdint>
#include <vector>
#include <thread>
#include "Game.h"
#include "Font.h"
#include "assets.h"

struct FrameTimer {
    std::chrono::time_point<std::chrono::high_resolution_clock> last_tick;

    FrameTimer() : last_tick(std::chrono::high_resolution_clock::now()) {}

    // Returns the time passed in seconds since the last call to delta()
    float delta() {
        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(now - last_tick).count();
        last_tick = now;
        return dt;
    }
};

inline void clear_screen(std::vector<uint32_t>& buf, uint32_t color) {
    std::fill(buf.begin(), buf.end(), color);
}

inline void draw_rectangle(std::vector<uint32_t>& buf, int rx, int ry, int rw, int rh, uint32_t color) {
    int start_x = std::max(0, rx), end_x = std::min(Game::WIDTH, rx + rw);
    int start_y = std::max(0, ry), end_y = std::min(Game::HEIGHT, ry + rh);
    for (int y = start_y; y < end_y; ++y) {
        for (int x = start_x; x < end_x; ++x) {
            buf[y * Game::WIDTH + x] = color;
        }
    }
}

// using export from https://www.pentacom.jp/pentacom/bitfontmaker2/
// which outputs weirdly right to left i think, mirrored,
// so the `draw_text` has to account for that.
// likely because it's a Japan website. good enough for basics for now
inline void draw_text(std::vector<uint32_t>& buf, int tx, int ty, const std::string& msg, uint32_t color, int scale = 1) {
    // Ensure scale is at least 1x
    if (scale < 1) scale = 1;

    int cursor_x = tx;
    constexpr int total_chars = sizeof(Font::MINI) / sizeof(Font::MINI[0]);

    for (char c : msg) {
        if (c == ' ') {
            // Scale the blank space gap proportionally
            cursor_x += 8 * scale;
            continue;
        }

        int idx = static_cast<unsigned char>(c) - 33;
        if (idx < 0 || idx >= total_chars) continue;

        // Loop over the base font rows
        for (int row = 0; row < Font::SIZE; ++row) {
            Font::RowType font_row = Font::MINI[idx][row];

            // Calculate where this block of pixels starts vertically
            int base_y = ty + (row * scale);

            // Loop over the base font columns
            for (int col = 0; col < Font::SIZE; ++col) {
                // If the bit is active, paint a scaled block!
                if ((font_row >> col) & 0x1) {
                    int base_x = cursor_x + (col * scale);

                    // Draw a 'scale x scale' square block of pixels on the screen
                    for (int sy = 0; sy < scale; ++sy) {
                        int current_y = base_y + sy;
                        if (current_y < 0 || current_y >= Game::HEIGHT) continue;

                        for (int sx = 0; sx < scale; ++sx) {
                            int current_x = base_x + sx;
                            if (current_x < 0 || current_x >= Game::WIDTH) continue;

                            buf[current_y * Game::WIDTH + current_x] = color;
                        }
                    }
                }
            }
        }

        // Move the typewriter cursor forward by the character size multiplied by the scale factor
        cursor_x += Font::SPACING * scale;
    }
}

inline void draw_sprite_rle(std::vector<uint32_t>& buf, int sx, int sy, int sw, int sh, const uint8_t* rle_data, uint16_t comp_size) {
    int px_idx = 0; uint16_t cursor = 0;
    while (cursor < comp_size) {
        uint8_t run = rle_data[cursor++]; uint8_t pal_idx = rle_data[cursor++];
        for (uint8_t i = 0; i < run; ++i) {
            int lx = px_idx % sw, ly = px_idx / sw; px_idx++;
            int tx = sx + lx, ty = sy + ly;
            if (tx >= 0 && tx < Game::WIDTH && ty >= 0 && ty < Game::HEIGHT && ly < sh) {
                uint32_t color = GLOBAL_PALETTE[pal_idx];
                if ((color & 0xFF000000) != 0x00000000) buf[ty * Game::WIDTH + tx] = color;
            }
        }
    }
}

inline bool present(struct mfb_window* window, std::vector<uint32_t>& pixel_buffer) {
    mfb_update_state status = mfb_update_ex(window, pixel_buffer.data(), Game::WIDTH, Game::HEIGHT);
    return (status == MFB_STATE_OK && mfb_wait_sync(window));
}

struct FrameLimiter {
    float frame_duration;

    FrameLimiter(float fps) : frame_duration(1.0f / fps) {}

    void wait(float dt) {
        if (dt < frame_duration) {
            float sleep_time = frame_duration - dt;
            // Sleep for the remainder of the frame duration
            std::this_thread::sleep_for(std::chrono::duration<float>(sleep_time));
        }
    }
};
