#pragma once

#include "esp_err.h"

struct Color
{
    uint8_t red, green, blue;
    Color();
    Color(uint8_t r, uint8_t g, uint8_t b);
    void setHSV(uint16_t hue, uint8_t saturation, uint8_t lightness);
};