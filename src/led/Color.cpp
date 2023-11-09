#include "Color.h"

#include <cmath>
#include <array>

#define HSV_SECTION_3 (0x40)

namespace led
{
    Color::Color() : red(0), green(0), blue(0)
    {
    }

    Color::Color(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b)
    {
    }

    inline double p(double v, double s)
    {
        return v * (1. - s);
    }

    inline double q(double v, double s, double f)
    {
        return v * (1. - s * f);
    }

    inline double t(double v, double s, double f)
    {
        return v * (1. - s * (1. - f));
    }

    void Color::setFromHSV(const int16_t hue, const double s, const double v)
    {
        if (s < 0.001)
        {
            this->red = this->green = this->blue = v * 255;
            return;
        }

        double interval = hue / 60.;
        int16_t hi = std::floor(interval);
        double f = interval - hi;

        double r{};
        double g{};
        double b{};

        switch (hi)
        {
        case 0:
        case 6:
            r = v;
            g = t(v, s, f);
            b = p(v, s);
            break;
        case 1:
            r = q(v, s, f);
            g = v;
            b = p(v, s);
            break;
        case 2:
            r = p(v, s);
            g = v;
            b = t(v, s, f);
            break;
        case 3:
            r = p(v, s);
            g = q(v, s, f);
            b = v;
            break;
        case 4:
            r = t(v, s, f);
            g = p(v, s);
            b = v;
            break;
        case 5:
            r = v;
            g = p(v, s);
            b = q(v, s, f);
            break;
        }

        this->red = r * 255;
        this->green = g * 255;
        this->blue = b * 255;
    }

    std::array<uint8_t, 3> Color::asGRB() const
    {
        std::array<uint8_t, 3> array{this->green, this->red, this->blue};
        return array;
    }
}