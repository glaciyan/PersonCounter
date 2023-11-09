#ifndef COLOR_H
#define COLOR_H

#include <memory>

namespace led
{
    struct Color final
    {
        uint8_t red, green, blue;
        Color();
        Color(uint8_t r, uint8_t g, uint8_t b);
        void setFromHSV(const int16_t hue, const double saturation, const double value);
        std::array<uint8_t, 3> asGRB() const;
    };
}

#endif