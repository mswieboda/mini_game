#include <algorithm>
#include "../Game.h"
#include "Draw.h"
#include "Font.h"
#include "assets.h"

namespace Draw {
    // NOTE: these are private, and invisible to public consumers
    namespace {
        std::vector<Command> g_queue;

        void clear_screen(std::vector<uint32_t>& buf, uint32_t color) {
            std::fill(buf.begin(), buf.end(), color);
        }

        void draw_rect_immediate(std::vector<uint32_t>& buf, int rx, int ry, int rw, int rh, uint32_t color, bool fill) {
            // TODO: `fill` unused for now until we impl outline etc
            int start_x = std::max(0, rx), end_x = std::min(Game::WIDTH, rx + rw);
            int start_y = std::max(0, ry), end_y = std::min(Game::HEIGHT, ry + rh);
            for (int y = start_y; y < end_y; ++y) {
                for (int x = start_x; x < end_x; ++x) {
                    buf[y * Game::WIDTH + x] = color;
                }
            }
        }

        void draw_text_immediate(std::vector<uint32_t>& buf, int x, int y, const std::string& text, uint32_t color, int scale) {
            // Ensure scale is at least 1x
            if (scale < 1) scale = 1;

            int cursor_x = x;
            constexpr int total_chars = sizeof(Font::MINI) / sizeof(Font::MINI[0]);

            for (char c : text) {
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
                    int base_y = y + (row * scale);

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

        void draw_sprite_immediate(std::vector<uint32_t>& buf, int x, int y, const uint8_t* pixel_data, uint16_t pixel_data_size, int width, int height) {
            int px_idx = 0; uint16_t cursor = 0;
            while (cursor < pixel_data_size) {
                // NOTE: pixel_data uses Run-Length Encoding (RLE).
                // Format: [run_length, palette_index] -> e.g., [100, 5] means "draw 100 pixels of palette color 5"
                uint8_t run = pixel_data[cursor++]; uint8_t pal_idx = pixel_data[cursor++];
                for (uint8_t i = 0; i < run; ++i) {
                    int lx = px_idx % width, ly = px_idx / width; px_idx++;
                    int tx = x + lx, ty = y + ly;
                    if (tx >= 0 && tx < Game::WIDTH && ty >= 0 && ty < Game::HEIGHT && ly < height) {
                        uint32_t color = GLOBAL_PALETTE[pal_idx];
                        if ((color & 0xFF000000) != 0x00000000) buf[ty * Game::WIDTH + tx] = color;
                    }
                }
            }
        }
    }

    void text(int x, int y, const std::string& text, uint32_t color, int scale, int z_index) {
        g_queue.push_back({ x, y, z_index, TextData{ text, color, scale } });
    }

    void rect(int x, int y, int width, int height, uint32_t color, bool fill, int z_index) {
        g_queue.push_back({ x, y, z_index, RectData{ width, height, color, fill } });
    }

    void sprite(int x, int y, const uint8_t* pixel_data, uint16_t pixel_data_size, int width, int height, int z_index) {
        g_queue.push_back({ x, y, z_index, SpriteData{ pixel_data, pixel_data_size, width, height } });
    }

    void flush_pipeline(std::vector<uint32_t>& buffer, uint32_t background_color) {
        clear_screen(buffer, background_color);

        // Sort the commands
        // Primary key: Z-Index (lower draws first).
        // Secondary key: Y coordinate (Classic 2.5D top-to-bottom depth layering)
        std::sort(g_queue.begin(), g_queue.end(), [](const Command& a, const Command& b) {
            if (a.z_index != b.z_index) {
                return a.z_index < b.z_index;
            }
            return a.y < b.y;
        });

        // Dispatch drawing functions using pattern matching
        for (const auto& cmd : g_queue) {
            std::visit([&](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, TextData>) {
                    draw_text_immediate(buffer, cmd.x, cmd.y, arg.text, arg.color, arg.scale);
                } 
                else if constexpr (std::is_same_v<T, RectData>) {
                    draw_rect_immediate(buffer, cmd.x, cmd.y, arg.width, arg.height, arg.color, arg.fill);
                } 
                else if constexpr (std::is_same_v<T, SpriteData>) {
                    draw_sprite_immediate(buffer, cmd.x, cmd.y, arg.pixel_data, arg.pixel_data_size, arg.width, arg.height);
                }
            }, cmd.data);
        }

        // Reset the vector size back to zero for the next frame,
        // but preserve the capacity so we don't trigger heap re-allocations
        g_queue.clear();
    }
}