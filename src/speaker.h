#ifndef __SPEAKER_H_
#define __SPEAKER_H_

#include <Arduino.h>
#include <cstdint>

// ----------------------------------------------------------------------------

#define _SPEAKER_CHANNEL_CAPACITY 12
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

static uint8_t _volume = 0;

static uint8_t _waveform = WAVEFORM_SAWTOOTH;

static int16_t _buffer[_SPEAKER_CHANNEL_CAPACITY] = {-1};
static int16_t _bufferSize = 0;

static int16_t _channels[_SPEAKER_CHANNEL_CAPACITY] = {-1};
static uint8_t _channelCount = 0;

// ----------------------------------------------------------------------------

unsigned fastModulo(uint32_t value, uint32_t divisor) {
    if (value - divisor >= divisor)
        value = fastModulo(value, divisor + divisor);
    while (value >= divisor)
        value -= divisor;
    return value;
}

/* Generates a sawtooth waveform

Arguments
---------
period:
    period of the note in microseconds
time:
    current epoch time in microseconds

Returns
-------
amplitude:
    waveform amplitude in the range [0:255]
*/
static uint8_t generateSawtooth(const int16_t period, const uint32_t time) {
	const uint32_t phase = time % period;
	return (phase << 7) / period + 127;
}

/* Generates a square waveform

Arguments
---------
period:
    period of the note in microseconds
time:
    current epoch time in microseconds

Returns
-------
amplitude:
    waveform amplitude in the range [0:255]
*/
static uint8_t generateSquare(const int16_t period, const uint32_t time) {
	const uint32_t phase = time % period;
    return (phase < period / 2) ? 0 : 255;
}

/* Generates a triangle waveform

Arguments
---------
period:
    period of the note in microseconds
time:
    current epoch time in microseconds

Returns
-------
amplitude:
    waveform amplitude in the range [0:255]
*/
static uint8_t generateTriangle(const int16_t period, const uint32_t time) {
	const uint32_t phase = time % period;
	uint32_t result = (phase << 9) / period;
    return (phase > (period / 2)) ? 512 - result : result;
}

/* Gets the period of a note in microseconds

Uses a cool lil' note period evaluating equation which works out the period
relative to A4 (= 440Hz). Gets less accurate the further away the note is from
this reference point

Arguments
---------
note:
    the note, the period of which to evaluate

Returns
-------
period:
    the period of the note in microseconds
*/
static int16_t notePeriod(const Note note) {
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

    return 1e6 / result;
}

/* Mixes channels together

Uses the "headroom" principle to sum the channels without clipping

Arguments
---------
channels:
    the channels to mix
count:
    the number of channels in the array

Returns
-------
amplitude:
    the resulting amplitude
*/
static uint8_t mixChannels(const uint8_t *channels, const uint8_t count) {
    uint16_t result = 0;
    for(uint8_t index = 0; index < count; index += 1)
        result += channels[index];
    return result / count;
}

/* Scales a signal

Arguments
---------
value:
    the value to scale
limit:
    the maximum amplitude of the resulting signal

Returns
-------
amplitude:
    the amplitude of the scaled signal
*/
static uint8_t scaleVolume(const uint8_t value, const uint8_t limit, const uint8_t count) {
    uint8_t limit_poly;
    if (count > 1) {
        limit_poly = limit + (count*12);
        if (limit_poly > 127) limit_poly = 127;
    }else limit_poly = limit;
    const int16_t delta = value - 127;
    const int16_t adjust = (delta * limit_poly) >> 8;
    return 127 + adjust;
}

/* Update ISR which handles playback

Generates a signal for each channel, mixes them together, and scales the
result
*/
void updateRoutine() {
    static uint32_t microsecondTime = 0;
    microsecondTime += 1e6 / _SPEAKER_UPDATE_FREQUENCY;

    if(_channelCount == 0) {
        microsecondTime = 0;
        return;
    }

    uint8_t values[_channelCount] = {0};
    for(uint8_t index = 0; index < _channelCount; index += 1) {
        const int16_t period = _channels[index];

        if(_waveform == WAVEFORM_SAWTOOTH)
            values[index] = generateSawtooth(period, microsecondTime);
        else if(_waveform == WAVEFORM_SQUARE)
            values[index] = generateSquare(period, microsecondTime);
        else if(_waveform == WAVEFORM_TRIANGLE)
            values[index] = generateTriangle(period, microsecondTime);
    }

    const uint8_t mixedChannels = mixChannels(values, _channelCount);
    const uint8_t amplitude = scaleVolume(mixedChannels, _volume, _channelCount);

    analogWrite(_SPEAKER_PIN_RIGHT, amplitude);
}

// ----------------------------------------------------------------------------

/* Initializes the speaker

Configures the update ISR and initializes speaker ports
*/
void speakerInitialize() {
    for(uint8_t index = 0; index < _SPEAKER_CHANNEL_CAPACITY; index += 1) {
        _buffer[index] = -1;
        _channels[index] = -1;
    }
    _bufferSize = 0;
    _channelCount = 0;

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

/* Sets the volume level

Arguments
---------
volume:
    the new volume level
*/
void speakerSetVolume(const uint8_t volume) {
    _volume = volume;
}

/* Sets the waveform

Options include:

- WAVEFORM_SAWTOOTH
- WAVEFORM_SQUARE
- WAVEFORM_TRIANGLE

Arguments
---------
waveform:
    the new waveform
*/
void speakerSetWaveform(const uint8_t waveform) {
    if(waveform != WAVEFORM_SAWTOOTH
            && waveform != WAVEFORM_SQUARE
            && waveform != WAVEFORM_TRIANGLE)
        return;

    _waveform = waveform;
}

/* Buffers a note to be played on next update

Arguments
---------
note:
    the note to play
*/
uint8_t speakerPlayNote(const Note note) {
    if(_bufferSize == _SPEAKER_CHANNEL_CAPACITY)
        return false;

    _buffer[_bufferSize] = notePeriod(note);
    _bufferSize += 1;

    return true;
}

/* Stops playing a note; takes effect on the next update

Arguments
---------
note:
    the note to stop playing
*/
uint8_t speakerStopNote(const Note note) {
    const int16_t period = notePeriod(note);

    uint8_t startIndex = 0;
    uint8_t found = false;
    for(startIndex = 0; startIndex < _bufferSize; startIndex += 1) {
        if(_buffer[startIndex] == period) {
            found = true;
            break;
        }
    }

    if(found == false)
        return false;

    _buffer[startIndex] = -1;
    for(uint8_t index = startIndex; index < (_bufferSize - 1); index += 1)
        _buffer[index] = _buffer[index + 1];
    
    return true;
}

// Stops playing all notes. Takes effect on the next update
void speakerStop() {
    for(uint8_t index = 0; index < _bufferSize; index += 1)
        _buffer[index] = -1;
    _bufferSize = 0;
}


// Pushes the buffered channels
void speakerUpdate() {
    const uint8_t maxSize = (_bufferSize > _channelCount)
            ? _bufferSize
            : _channelCount;

    _channelCount = _bufferSize;
    for(uint8_t index = 0; index < maxSize; index += 1)
        __atomic_store_n(&(_channels[index]), 
                _buffer[index], 
                __ATOMIC_RELAXED);

    // for(uint8_t index = 0; index < _channelCount; index += 1) {
    //     Serial.printf("%d%s", 
    //             _channels[index], 
    //             ((index + 1) < maxSize) ? ", " : "\n");
    // }
}

#endif // __SPEAKER_H_
