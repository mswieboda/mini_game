#pragma once

#include <cstdint>

struct FontData {
    int size = 16;
    int spacing = 10;
    using RowType = uint16_t;

    // 128 ASCII chars, up to 16 rows per char
    RowType data[128][16] = {}; // Guaranteed all zeros
};

namespace Font {
    inline constexpr FontData DEFAULT_BLANK = FontData{};
}
