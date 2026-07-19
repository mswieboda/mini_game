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

inline void draw_text(std::vector<uint32_t>& buf, int tx, int ty, const std::string& msg, uint32_t color) {
    int cursor_x = tx;
    for (char c : msg) {
        // Use your new FONT_ORDER to find the index
        const char* p = strchr(FONT_ORDER, toupper(c));
        if (!p) continue;
        int idx = (int)(p - FONT_ORDER);

        for (int row = 0; row < 8; ++row) {
            uint8_t font_row = MINI_FONT[idx][row];
            int current_y = ty + row;
            if (current_y < 0 || current_y >= Game::HEIGHT) continue;
            for (int col = 0; col < 8; ++col) {
                int current_x = cursor_x + col;
                if (current_x < 0 || current_x >= Game::WIDTH) continue;
                if ((font_row >> (7 - col)) & 0x1)
                    buf[current_y * Game::WIDTH + current_x] = color;
            }
        }
        cursor_x += 8;
    }
}

inline void draw_sprite_rle(std::vector<uint32_t>& buf, int sx, int sy, int sw, int sh, const uint8_t* rle_data, uint16_t comp_size) {
    int px_idx = 0; uint16_t cursor = 0;
    while (cursor < comp_size) {
        uint8_t run = rle_data[cursor++]; uint8_t pal_idx = rle_data[cursor++];
        for (uint8_t i = 0; i < run; ++i) {
            int lx = px_idx % sw, ly = px_idx / sw; px_idx++;
            int tx = sx + lx, ty = sy + ly;
            if (tx >= 0 && tx < Game::WIDTH && ty >= 0 && ty < Game::HEIGHT) {
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
