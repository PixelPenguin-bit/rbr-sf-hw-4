/**
 * @file main.cpp
 * @author Codex for Ashwin Raj
 * @brief Blinks the board LED and adjusts the cycle and duty rate with buttons.
 * @version 1.0.0
 * @date 2026-11-22
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "board.h"

namespace
{
constexpr uint32_t cycleStepMs = 100;
constexpr uint32_t maxCycleMs = UINT32_MAX - UINT32_MAX % cycleStepMs;
constexpr uint32_t debounceMs = 20;

struct Button
{
    explicit Button(uint8_t inputPin) : pin(inputPin) {}

    bool wasPressed(uint32_t now)
    {
        const bool reading = digitalRead(pin) == BUTTON_ACTIVE;
        if (reading != rawPressed)
        {
            rawPressed = reading;
            lastChangeMs = now;
        }

        if (rawPressed != stablePressed && now - lastChangeMs >= debounceMs)
        {
            stablePressed = rawPressed;
            return stablePressed;
        }
        return false;
    }

    const uint8_t pin;
    bool rawPressed = false;
    bool stablePressed = false;
    uint32_t lastChangeMs = 0;
};

Button slower(BUTTON_1);
Button faster(BUTTON_2);
Button duty(BUTTON_3);
uint32_t cycleMs = 1000;
uint32_t dutyPercent = 50;
uint32_t cycleStartMs = 0;
}

/** Configures the LED, the three active-low inputs, and the cycle clock. */
void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(BUTTON_1, BUTTON_MODE);
    pinMode(BUTTON_2, BUTTON_MODE);
    pinMode(BUTTON_3, BUTTON_MODE);
    cycleStartMs = millis();
}

/** Updates the button states and LED without blocking the next poll. */
void loop()
{
    const uint32_t now = millis();

    if (slower.wasPressed(now) && cycleMs < maxCycleMs)
    {
        cycleMs += cycleStepMs;
    }
    if (faster.wasPressed(now) && cycleMs > cycleStepMs)
    {
        cycleMs -= cycleStepMs;
    }
    if (duty.wasPressed(now))
    {
        dutyPercent = (dutyPercent + 25) % 100;
    }

    // Keep an unsigned cycle origin so subtraction works across millis() rollover.
    const uint32_t phaseMs = (now - cycleStartMs) % cycleMs;
    cycleStartMs = now - phaseMs;

    // The period is a multiple of 100 ms. Dividing first avoids overflow.
    const uint32_t onMs = (cycleMs / 100) * dutyPercent;
    digitalWrite(LED_BUILTIN, phaseMs < onMs ? HIGH : LOW);
}
