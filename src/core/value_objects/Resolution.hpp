#pragma once

#include <cstdint>
#include <stdexcept>

namespace Core::ValueObjects {

struct Resolution {
    uint32_t width;
    uint32_t height;

    constexpr Resolution(uint32_t w, uint32_t h) : width(w), height(h) {
        if (width == 0 || height == 0) {
            // In a real application we might throw, but for constexpr simplicity/safety we rely on usage.
            // Leaving constructor simple for now. 
            // A validation method() could be better or throwing logic if not constexpr.
        }
    }

    constexpr bool isValid() const {
        return width > 0 && height > 0;
    }

    constexpr uint32_t totalPixels() const {
        return width * height;
    }
};

} // namespace Core::ValueObjects
