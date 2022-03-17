#ifndef __SPEAKER_H_
#define __SPEAKER_H_

#include <Arduino.h>
#include <cstdint>

// ----------------------------------------------------------------------------

#define _SPEAKER_CHANNEL_LIMIT 12
#define _SPEAKER_UPDATE_PERIOD 22000

// ----------------------------------------------------------------------------

#define _SPEAKER_PIN_LEFT  A4
#define _SPEAKER_PIN_RIGHT A3

// ----------------------------------------------------------------------------

#define WAVEFORM_SAWTOOTH 0
#define WAVEFORM_SQUARE   1
#define WAVEFORM_TRIANGLE 2

// ----------------------------------------------------------------------------

#define NOTE_C       -9
#define NOTE_C_SHARP -8
#define NOTE_D_FLAT  -8
#define NOTE_D       -7
#define NOTE_D_SHARP -6
#define NOTE_E_FLAT  -6
#define NOTE_E       -5
#define NOTE_F       -4
#define NOTE_F_SHARP -3
#define NOTE_G_FLAT  -3
#define NOTE_G       -2
#define NOTE_G_SHARP -1
#define NOTE_A_FLAT  -1
#define NOTE_A        0
#define NOTE_A_SHARP  1
#define NOTE_B_FLAT   1
#define NOTE_B        2

// ----------------------------------------------------------------------------

static uint8_t volume = 0;

static uint8_t waveform = WAVEFORM_SAWTOOTH;

static uint8_t channelCount = 0;
static int16_t pitches[_SPEAKER_CHANNEL_LIMIT] = {-1};

// ----------------------------------------------------------------------------

static uint8_t generateSawtooth(const int16_t pitch, const uint32_t time) {
	const uint32_t period = 1e6 / pitch;
	const uint32_t phase = time % period;
	return (phase << 7) / period + 127;
}

static uint8_t generateSquare(const int16_t pitch, const uint32_t time) {
	const uint32_t period = 1e6 / pitch;
	const uint32_t phase = time % period;
    return (phase < period / 2) ? 0 : 255;
}

static uint8_t generateTriangle(const int16_t pitch, const uint32_t time) {
	const uint32_t period = 1e6 / pitch;
	const uint32_t phase = time % period;
	uint32_t result = (phase << 9) / period;
    return (phase > (period / 2)) ? 512 - result : result;
}

static int16_t notePitch(const uint8_t note, const uint8_t octave) {
    uint32_t result = 440;

    const int8_t octaveDelta = octave - 4;
    int16_t steps = note + (octaveDelta * 11);

    const uint8_t positive = steps > 0;
    steps = (steps > 0) ? steps : -steps;

    result <<= 3;
    for(uint8_t step = 0; step < steps; step += 1) {
        result *= positive ? 1059463 : 1e6;
        result /= positive ? 1e6 : 1059463;
    }
    result >>= 3;

    return result;
}

static uint8_t mix(const uint8_t *pitches, const uint8_t channelCount) {
    uint16_t result = 0;
    for(uint8_t index = 0; index < channelCount; index += 1)
        result += pitches[index];
    return result / channelCount;
}

static uint8_t scaleVolume(const uint8_t value, const uint8_t volume) {
    const int16_t delta = value - 127;
    const int16_t adjust = (delta * volume) >> 8;
    return 127 + adjust;
}

void updateRoutine() {
    static uint32_t microsecondTime = 0;
    microsecondTime += _SPEAKER_UPDATE_PERIOD;

    uint8_t channels[channelCount] = {0};
    uint8_t channelIndex = 0;
    for(uint8_t pitchIndex = 0;
            pitchIndex < _SPEAKER_CHANNEL_LIMIT;
            pitchIndex += 1) {

        if(pitches[pitchIndex] == -1)
            continue;

        uint8_t value = 0;
        const uint8_t pitch = pitches[pitchIndex];
        if(waveform == WAVEFORM_SAWTOOTH)
            value = generateSawtooth(pitch, microsecondTime);
        else if(waveform == WAVEFORM_SQUARE)
            value = generateSquare(pitch, microsecondTime);
        else if(waveform == WAVEFORM_TRIANGLE)
            value = generateTriangle(pitch, microsecondTime);

        channels[channelIndex] = value;
        channelIndex += 1;
    }

    const uint8_t mixedChannels = mix(channels, channelCount);
    const uint8_t amplitude = scaleVolume(mixedChannels, volume);
    analogWrite(_SPEAKER_PIN_RIGHT, amplitude);
}

// ----------------------------------------------------------------------------

void speakerInitialize() {
    pinMode(_SPEAKER_PIN_LEFT, OUTPUT);
    pinMode(_SPEAKER_PIN_RIGHT, OUTPUT);

    TIM_TypeDef *handle = TIM1;
    HardwareTimer *timer = new HardwareTimer(handle);

    timer->setOverflow(_SPEAKER_UPDATE_PERIOD, HERTZ_FORMAT);
    timer->attachInterrupt(updateRoutine);
    timer->resume();
}

void speakerSetVolume(const uint8_t volume_) {
    volume = volume_;
}

void speakerSetWaveform(const uint8_t waveform_) {
    if(waveform != WAVEFORM_SAWTOOTH
            && waveform != WAVEFORM_SQUARE
            && waveform != WAVEFORM_TRIANGLE)
        return;

    waveform = waveform_;
}

uint8_t speakerPlayNote(const int8_t note, const uint8_t octave) {

    const int16_t pitch = notePitch(note, octave);

    uint8_t found = 0;
    for(uint8_t index = 0; index < _SPEAKER_CHANNEL_LIMIT; index += 1) {

        // If there's an empty pitch slot and one hasn't been found yet, fill
        // that one
        if(pitches[index] == -1 && !found) {
            pitches[index] = pitch;
            found = 1;
        }

        // Otherwise if the pitch is encountered (possibly for the 2nd time),
        // clear *that* slot
        else if(pitches[index] == pitch) {
            if(found)
                pitches[index] = -1;
            else
                found = 1;
        }
    }

    channelCount += found;
    return found;
}

void speakerStopNote(const int8_t note, const uint8_t octave) {
    for(uint8_t index = 0; index < _SPEAKER_CHANNEL_LIMIT; index += 1) {
        if(pitches[index] == note) {
            pitches[index] = -1;
            channelCount -= 1;
            return;
        }
    }
}

void speakerStopAll() {
    for(uint8_t index = 0; index < _SPEAKER_CHANNEL_LIMIT; index += 1)
        pitches[index] = -1;
    channelCount = 0;
}

#endif // __SPEAKER_H_
