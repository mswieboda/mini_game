#pragma once
#include <string>

namespace Log {
    // Standard text logger
    void msg(const std::string& message);
    
    // Formatted helper for quickly printing text with numbers (like key codes)
    void fmt(const char* format, ...);
}
