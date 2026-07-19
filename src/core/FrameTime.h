#pragma once
#include <chrono>

class FrameTime {
private:
    double m_target_fps;
    double m_time_step;
    double m_accumulator;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_clock_prev;

public:
    FrameTime(double target_fps) 
        : m_target_fps(target_fps), 
          m_time_step(1.0 / target_fps), 
          m_accumulator(0.0),
          m_clock_prev(std::chrono::high_resolution_clock::now()) {}

    // Updates the clock, returns true if we need to run a physics/logic step
    bool tick() {
        auto clock_now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = clock_now - m_clock_prev;
        m_clock_prev = clock_now;

        double frame_time = elapsed.count();
        if (frame_time > 0.1) frame_time = 0.1; // Spiral of death guard
        
        m_accumulator += frame_time;
        return m_accumulator >= m_time_step;
    }

    // Consumes one step fraction from the accumulator pool
    void consume_step() {
        m_accumulator -= m_time_step;
    }

    // Read-only access to our rigid target delta time
    double fixed_delta() const { return m_time_step; }
};
