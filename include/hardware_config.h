#pragma once

#include <Arduino.h>

namespace hardware
{
    constexpr uint8_t boardLedPin = 2;

    constexpr uint8_t ledOutputPins[] = {26, 27, 18, 19, 23, 13, 16, 17};

    constexpr uint8_t encoderClockPin = 33;
    constexpr uint8_t encoderDataPin = 34;
    constexpr uint8_t encoderSwitchPin = 35;

    constexpr uint8_t initialTubesPerOutput = 1;
    constexpr uint8_t maximumTubesPerOutput = 3;
}