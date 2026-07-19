#include <MiniFB.h>

namespace Input {
    inline constexpr int MAX_KEYS = 512;

    extern bool current_keys[MAX_KEYS];
    extern bool previous_keys[MAX_KEYS];

    void keyboard_callback(struct mfb_window *window, mfb_key key, mfb_key_mod mod, bool is_pressed);
    void update_input_state();

    bool is_key_pressed(int key);
    bool is_key_just_pressed(int key);
}
