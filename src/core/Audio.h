#pragma once
#include <vector>
#include <cstdint>
#include <cmath>

enum WaveType {
    SQUARE = 0,
    SAWTOOTH,
    SINE,
    NOISE
};

struct SfxrParams {
    WaveType wave_type = SQUARE;

    // Envelopes (in seconds)
    float attack_time = 0.0f;
    float sustain_time = 0.1f;
    float decay_time = 0.2f;

    // Frequency / Pitch
    float start_frequency = 0.3f;
    float min_frequency = 0.0f;
    float slide = 0.0f;           // Pitch sweep rate

    // Vibrato
    float vibrato_depth = 0.0f;
    float vibrato_speed = 0.0f;

    // Square wave duty cycle
    float square_duty = 0.5f;
};

namespace Audio {
    bool init();
    void cleanup();

    // Triggers an SFXR sound effect
    void play_sfx(const SfxrParams& params);
}
