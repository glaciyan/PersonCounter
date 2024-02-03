#pragma once

#include "led/Color.h"

#define PULSE_LENGHT_TICKS 100

enum LEDAnimation : int
{
    CLEAR = -1,
    FADE = 0,
    PULSE = 1
};

class Animation
{
public:
    LEDAnimation type;
    led::Color color;

    Animation(LEDAnimation type, led::Color color);
};