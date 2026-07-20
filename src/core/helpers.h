#pragma once
#include <cstdint>
#include <vector>
#include <thread>
#include <chrono>

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

inline uint32_t blend_pixel(uint32_t dest, uint32_t src) {
    uint32_t src_alpha = (src >> 24) & 0xFF;
    if (src_alpha == 0xFF) return src;
    if (src_alpha == 0x00) return dest;

    uint32_t src_r = (src >> 16) & 0xFF;
    uint32_t src_g = (src >> 8) & 0xFF;
    uint32_t src_b = src & 0xFF;

    uint32_t dest_alpha = (dest >> 24) & 0xFF;
    uint32_t dest_r = (dest >> 16) & 0xFF;
    uint32_t dest_g = (dest >> 8) & 0xFF;
    uint32_t dest_b = dest & 0xFF;

    uint32_t out_r = (src_r * src_alpha + dest_r * (255 - src_alpha)) / 255;
    uint32_t out_g = (src_g * src_alpha + dest_g * (255 - src_alpha)) / 255;
    uint32_t out_b = (src_b * src_alpha + dest_b * (255 - src_alpha)) / 255;
    uint32_t out_a = src_alpha + (dest_alpha * (255 - src_alpha)) / 255;

    return (out_a << 24) | (out_r << 16) | (out_g << 8) | out_b;
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
