#include <Arduino.h>

namespace Speaker {

enum Pins {
    LEFT = A4,
    RIGHT = A3,
};

enum Waveform {
    SAWTOOTH,
    SQUARE,
    TRIANGLE,
    SINE,
};

enum Key {
    C       = -9,
    C_SHARP = -8,
    D_FLAT  = -8,
    D       = -7,
    D_SHARP = -6,
    E_FLAT  = -6,
    E       = -5,
    F       = -4,
    F_SHARP = -3,
    G_FLAT  = -3,
    G       = -2,
    G_SHARP = -1,
    A_FLAT  = -1,
    A       = 0,
    A_SHARP = 1,
    B_FLAT  = 1,
    B       = 2,
};

static Waveform waveform = Waveform::SAWTOOTH;

static const uint16_t updatePeriod = 22000;

static uint32_t pitch = 0;

uint32_t keyPitch(const Key &key, const uint8_t &octave) {
    uint32_t result = 440;

    const int8_t octaveDelta = octave - 4;
    int16_t steps = key + octaveDelta * 11;

    const bool positive = steps > 0;
    steps = abs(steps);

    for(uint8_t step = 0; step < steps; step += 1) {
        result *= positive ? 1059463 : 1e6;
        result /= positive ? 1e6 : 1059463;
    }

    return result;
}

void updateRoutine() {
    static uint32_t microseconds = 0;
    microseconds += updatePeriod;

    const uint32_t period = 1e6 / pitch;

    static bool sign = false;
    while(microseconds > (period / 2)) {
        microseconds -= (period / 2);
        sign = !sign;
        analogWrite(Pins::RIGHT, 128 + (sign ? 127 : 0));
    }

    static uint32_t ticks = 0;
    ticks += 1;
    if(ticks > updatePeriod) {
        ticks -= updatePeriod;
        Serial.printf("%d\n", pitch);
    }
}

void initialize() {
    pinMode(Pins::LEFT, OUTPUT);
    pinMode(Pins::RIGHT, OUTPUT);

    TIM_TypeDef *handle = TIM1;
    HardwareTimer *timer = new HardwareTimer(handle);

    timer->setOverflow(updatePeriod, HERTZ_FORMAT);
    timer->attachInterrupt(updateRoutine);
    timer->resume();

    pitch = keyPitch(Key::C, 4);
}

};
