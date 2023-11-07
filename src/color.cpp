#include "color.h"

#define HSV_SECTION_3 (0x40)

// https://github.com/FastLED/FastLED/blob/3a03742a09aeb219a065954d7f85be9cdd2582f0/src/hsv2rgb.cpp#L41
void hsv2rgb(const int32_t hue, const int8_t saturation, const int8_t value, uint8_t &red, uint8_t &green, uint8_t &blue)
{
    uint8_t invsat = 255 - saturation;
    uint8_t brightness_floor = (value * invsat) / 256;
    uint8_t color_amplitude = value - brightness_floor;

    uint8_t section = hue / HSV_SECTION_3; // 0..2
    uint8_t offset = hue % HSV_SECTION_3;  // 0..63

    uint8_t rampup = offset;                         // 0..63
    uint8_t rampdown = (HSV_SECTION_3 - 1) - offset; // 63..0

    uint8_t rampup_amp_adj = (rampup * color_amplitude) / (256 / 4);
    uint8_t rampdown_amp_adj = (rampdown * color_amplitude) / (256 / 4);

    uint8_t rampup_adj_with_floor = rampup_amp_adj + brightness_floor;
    uint8_t rampdown_adj_with_floor = rampdown_amp_adj + brightness_floor;

    if (section)
    {
        if (section == 1)
        {
            // section 1: 0x40..0x7F
            red = brightness_floor;
            green = rampdown_adj_with_floor;
            blue = rampup_adj_with_floor;
        }
        else
        {
            // section 2; 0x80..0xBF
            red = rampup_adj_with_floor;
            green = brightness_floor;
            blue = rampdown_adj_with_floor;
        }
    }
    else
    {
        // section 0: 0x00..0x3F
        red = rampdown_adj_with_floor;
        green = rampup_adj_with_floor;
        blue = brightness_floor;
    }
}

Color::Color() : red(0), green(0), blue(0)
{
}

Color::Color(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b)
{
}

void Color::setHSV(uint16_t hue, uint8_t saturation, uint8_t value)
{
    hsv2rgb(hue, saturation, value, this->red, this->green, this->blue);
}
