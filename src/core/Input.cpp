#include "Input.h"
#include <cstring>
#include <MiniFB.h>
#include <algorithm>

namespace Input {
    bool current_keys[MAX_KEYS] = { false };
    bool previous_keys[MAX_KEYS] = { false };

    void update_input_state() {
        std::copy(std::begin(current_keys), std::end(current_keys), previous_keys);
    }

    void keyboard_callback(struct mfb_window *window, mfb_key key, mfb_key_mod mod, bool is_pressed) {
        // Cast to int to perform a valid range check against MAX_KEYS safely
        const int key_code = static_cast<int>(key);

        if (key_code >= 0 && key_code < MAX_KEYS) {
            current_keys[key_code] = is_pressed;
        }
    }

    bool is_key_pressed(int key) {
        if (key < 0 || key >= MAX_KEYS) return false;

        return current_keys[key];
    }

    bool is_key_just_pressed(int key) {
        if (key < 0 || key >= MAX_KEYS) return false;

        return current_keys[key] && !previous_keys[key];
    }
}
