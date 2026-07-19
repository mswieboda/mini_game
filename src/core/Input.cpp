#include "Input.h"
#include <cstring>
#include <MiniFB.h>

#include <iostream> // for cout, we can remove this with removal of debug logs

namespace Input {
    bool current_keys[MAX_KEYS] = { false };
    bool previous_keys[MAX_KEYS] = { false };

    void keyboard_callback(struct mfb_window *window, mfb_key key, mfb_key_mod mod, bool is_pressed) {
        // Cast to int to perform a valid range check against MAX_KEYS safely
        const int key_code = static_cast<int>(key);

        if (key_code >= 0 && key_code < MAX_KEYS) {
            current_keys[key_code] = is_pressed;
        }
    }

    void update_input_state() {
        // memcpy(previous_keys, current_keys, sizeof(current_keys));
    }

    bool is_key_pressed(int key) {
        if (key < 0 || key >= MAX_KEYS) return false;

        return current_keys[key];
    }

    bool is_key_just_pressed(int key) {
        if (key < 0 || key >= MAX_KEYS) return false;

        // If it's currently down, and our history says it WASN'T down yet:
        if (current_keys[key] && !previous_keys[key]) {
            previous_keys[key] = true; // Instantly lock it out so it can't trigger again this press
            return true;
        }

        // If the key was physically released, reset our history lock
        if (!current_keys[key]) {
            previous_keys[key] = false;
        }

        return false;
    }
}
