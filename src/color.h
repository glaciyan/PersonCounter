#pragma once

#include "esp_err.h"

struct Color
{
    uint8_t red, green, blue;
    Color();
    Color(uint8_t r, uint8_t g, uint8_t b);
    void setFromHSV(const int16_t hue, const double saturation, const double value);
};