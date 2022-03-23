#ifndef __SPEAKER_H_
#define __SPEAKER_H_

#include <Arduino.h>
#include <cstdint>

// ----------------------------------------------------------------------------

#define _SPEAKER_CHANNEL_COUNT    12
#define _SPEAKER_UPDATE_FREQUENCY 22000

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

struct Note {

    int8_t value;

    uint8_t octave;

};

Note createNote(const int8_t value, const uint8_t octave) {
    Note result;
    result.value = value;
    result.octave = octave;
    return result;
}

// ----------------------------------------------------------------------------

static uint8_t volume = 0;

static uint8_t waveform = WAVEFORM_SAWTOOTH;

volatile static uint8_t channelCount = 0;
volatile static int16_t pitches[_SPEAKER_CHANNEL_COUNT] = {-1};

// volatile static int16_t pitch = -1;

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

static int16_t notePitch(const Note note) {
    uint32_t result = 440;

    const int8_t octaveDelta = note.octave - 4;
    int16_t steps = note.value + (octaveDelta * 11);

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

static uint8_t mix(const uint8_t *pitches, const uint8_t count) {
    uint16_t result = 0;
    for(uint8_t index = 0; index < count; index += 1)
        result += pitches[index];
    return result / count;
}

static uint8_t scaleVolume(const uint8_t value, const uint8_t limit) {
    const int16_t delta = value - 127;
    const int16_t adjust = (delta * limit) >> 8;
    return 127 + adjust;
}

void updateRoutine() {
    static uint32_t microsecondTime = 0;
    microsecondTime += 1e6 / _SPEAKER_UPDATE_FREQUENCY;

    if(channelCount == 0) {
        microsecondTime = 0;
        return;
    }

    uint8_t values[channelCount] = {0};
    uint8_t valueIndex = 0;
    for(uint8_t pitchIndex = 0; 
            pitchIndex < _SPEAKER_CHANNEL_COUNT; 
            pitchIndex += 1) {
        
        if(valueIndex == channelCount)
            break;

        const int16_t pitch = pitches[pitchIndex];
        if(pitch == -1)
            continue;
        
        uint8_t value = 0;
        if(waveform == WAVEFORM_SAWTOOTH)
            value = generateSawtooth(pitch, microsecondTime);
        else if(waveform == WAVEFORM_SQUARE)
            value = generateSquare(pitch, microsecondTime);
        else if(waveform == WAVEFORM_TRIANGLE)
            value = generateTriangle(pitch, microsecondTime);
        values[valueIndex] = value;
    }

    const uint8_t mixedChannels = mix(values, channelCount);
    const uint8_t amplitude = scaleVolume(mixedChannels, volume);
    analogWrite(_SPEAKER_PIN_RIGHT, amplitude);
}

// ----------------------------------------------------------------------------

void speakerInitialize() {
    for(uint8_t index = 0; index < _SPEAKER_CHANNEL_COUNT; index += 1)
        pitches[index] = -1;
    channelCount = 0;

    pinMode(_SPEAKER_PIN_LEFT, OUTPUT);
    pinMode(_SPEAKER_PIN_RIGHT, OUTPUT);

    analogWrite(_SPEAKER_PIN_LEFT, 128);
    analogWrite(_SPEAKER_PIN_RIGHT, 128);

    TIM_TypeDef *handle = TIM1;
    HardwareTimer *timer = new HardwareTimer(handle);

    timer->setOverflow(_SPEAKER_UPDATE_FREQUENCY, HERTZ_FORMAT);
    timer->attachInterrupt(updateRoutine);
    timer->resume();
}

void speakerSetVolume(const uint8_t volume_) {
    volume = volume_;
}

void speakerSetWaveform(const uint8_t waveform_) {
    if(waveform_ != WAVEFORM_SAWTOOTH
            && waveform_ != WAVEFORM_SQUARE
            && waveform_ != WAVEFORM_TRIANGLE)
        return;

    waveform = waveform_;
}

uint8_t speakerPlayNote(const Note note) {
    uint8_t index = 0;
    for(index = 0; index < _SPEAKER_CHANNEL_COUNT; index += 1) {
        if(pitches[index] == -1)
            break;
    }

    if(pitches[index] != -1)
        return false;
    
    pitches[index] = notePitch(note);
    channelCount += 1;
    return true;
}

uint8_t speakerPlayNotes(const Note *notes, const uint8_t count) {
    uint8_t success = 1;
    for(uint8_t index = 0; index < count; index += 1)
        success &= speakerPlayNote(notes[index]);
    return success;
}

uint8_t speakerSetNotes(const Note *notes, const uint8_t count) {
    for(uint8_t index = 0; index < _SPEAKER_CHANNEL_COUNT; index += 1) {
        if(index < count) {
            const int16_t pitch = notePitch(notes[index]);
            pitches[index] = pitch;
        }
        else
            pitches[index] = 0;
    }
}

void speakerStopAll() {
    for(uint8_t index = 0; index < _SPEAKER_CHANNEL_COUNT; index += 1)
        pitches[index] = -1;
    channelCount = 0;
}

#endif // __SPEAKER_H_
