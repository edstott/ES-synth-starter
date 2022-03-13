#pragma once

#include <Arduino.h>

namespace Speaker {

enum Pins {
    LEFT = A4,
    RIGHT = A3,
};

enum Waveform {
    SAWTOOTH,
    SINE,
    SQUARE,
    TRIANGLE,
};

static struct State {

public:

    uint16_t pitch = 0;
    uint8_t volume = 0;

    Waveform waveform;

    bool playing;

    State() : pitch(0),
            volume(0),
            waveform(Waveform::SAWTOOTH),
            playing(false) {}

} state;

static const uint16_t updateFrequency = 22000;

static uint8_t generateSawtooth(const float &time, 
        const uint16_t &pitch, 
        const uint16_t &volume);
static uint8_t generateSine(const float &time, 
        const uint16_t &pitch, 
        const uint16_t &volume);
static uint8_t generateSquare(const float &time, 
        const uint16_t &pitch, 
        const uint16_t &volume);
static uint8_t generateTriangle(const float &time,
        const uint16_t &pitch, 
        const uint16_t &volume);

static void updateRoutine();

void initialize();

};
