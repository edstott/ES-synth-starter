#ifndef __SPEAKER_H_
#define __SPEAKER_H_

#include <Arduino.h>
#include <cstdint>

// ----------------------------------------------------------------------------

#define _SPEAKER_CHANNEL_CAPACITY    12
#define _SPEAKER_UPDATE_FREQUENCY 22000

// ----------------------------------------------------------------------------

#define _SPEAKER_PIN_LEFT  A4
#define _SPEAKER_PIN_RIGHT A3

// ----------------------------------------------------------------------------

#define _SPEAKER_BUFFER  0
#define _SPEAKER_CURRENT 1

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

static int16_t _channels[_SPEAKER_CHANNEL_CAPACITY];
static int16_t _channelsBuffer[_SPEAKER_CHANNEL_CAPACITY];

static uint8_t _channelCount = 0;
static uint8_t _channelCountBuffer = 0;

// ----------------------------------------------------------------------------

/* Generates a sawtooth waveform

Arguments
---------
pitch:
    frequency of the note in Hertz
time:
    current epoch time in microseconds

Returns
-------
amplitude:
    waveform amplitude in the range [0:255]
*/
static uint8_t generateSawtooth(const int16_t pitch, const uint32_t time) {
	const uint32_t period = 1e6 / pitch;
	const uint32_t phase = time % period;
	return (phase << 7) / period + 127;
}

/* Generates a square waveform

Arguments
---------
pitch:
    frequency of the note in Hertz
time:
    current epoch time in microseconds

Returns
-------
amplitude:
    waveform amplitude in the range [0:255]
*/
static uint8_t generateSquare(const int16_t pitch, const uint32_t time) {
	const uint32_t period = 1e6 / pitch;
	const uint32_t phase = time % period;
    return (phase < period / 2) ? 0 : 255;
}

/* Generates a triangle waveform

Arguments
---------
pitch:
    frequency of the note in Hertz
time:
    current epoch time in microseconds

Returns
-------
amplitude:
    waveform amplitude in the range [0:255]
*/
static uint8_t generateTriangle(const int16_t pitch, const uint32_t time) {
	const uint32_t period = 1e6 / pitch;
	const uint32_t phase = time % period;
	uint32_t result = (phase << 9) / period;
    return (phase > (period / 2)) ? 512 - result : result;
}

/* Gets the pitch of a note in Hertz

Uses a cool lil' note pitch evaluating equation which works out the pitch
relative to A4 (= 440Hz). Gets less accurate the further away the note is from
this reference point

Arguments
---------
note:
    the note, the pitch of which to evaluate

Returns
-------
pitch:
    the pitch of the note in Hertz
*/
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

/* Mixes channels together

Uses the "headroom" principle to sum the channels without clipping

Arguments
---------
pitches:
    the pitches to mix
count:
    the number of channels in the array

Returns
-------
amplitude:
    the resulting amplitude
*/
static uint8_t mixChannels(const uint8_t *pitches, const uint8_t count) {
    uint16_t result = 0;
    for(uint8_t index = 0; index < count; index += 1)
        result += pitches[index];
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
static uint8_t scaleVolume(const uint8_t value, const uint8_t limit) {
    const int16_t delta = value - 127;
    const int16_t adjust = (delta * limit) >> 8;
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
    uint8_t valueIndex = 0;
    for(uint8_t index = 0; index < _SPEAKER_CHANNEL_CAPACITY; index += 1) {
        if(valueIndex == _channelCount)
            break;

        const int16_t pitch = _channels[index];
        if(pitch == -1)
            continue;

        uint8_t value = 0;
        if(_waveform == WAVEFORM_SAWTOOTH)
            value = generateSawtooth(pitch, microsecondTime);
        else if(_waveform == WAVEFORM_SQUARE)
            value = generateSquare(pitch, microsecondTime);
        else if(_waveform == WAVEFORM_TRIANGLE)
            value = generateTriangle(pitch, microsecondTime);

        values[valueIndex] = value;
        valueIndex += 1;
    }

    const uint8_t mixedChannels = mixChannels(values, _channelCount);
    const uint8_t amplitude = scaleVolume(mixedChannels, _volume);
    analogWrite(_SPEAKER_PIN_RIGHT, amplitude);
}

// ----------------------------------------------------------------------------

/* Initializes the speaker

Configures the update ISR and initializes speaker ports
*/
void speakerInitialize() {
    for(uint8_t index = 0; index < _SPEAKER_CHANNEL_CAPACITY; index += 1) {
        _channelBuffer[index] = -1;
        _channels[index] = -1;
    }
    _channelCountBuffer = 0;
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
    const int16_t pitch = notePitch(note);

    uint8_t found = 0;
    for(uint8_t index = 0; index < _SPEAKER_CHANNEL_CAPACITY; index += 1) {
        if(_channelBuffer[index] == -1 && found == false) {
            _channelBuffer[index] = pitch;
            _channelCountBuffer += 1;
            found = true;
        }

        else if(_channelBuffer[index] == pitch) {
            if(found)
                _channelBuffer[index] = -1;
            else
                found = true;
        }
    }

    return found;
}

/* Stops playing a note; takes effect on the next update

Arguments
---------
note:
    the note to stop playing
*/
uint8_t speakerStopNote(const Note note) {
    const int16_t pitch = notePitch(note);
    for(uint8_t index = 0; index < _SPEAKER_CHANNEL_CAPACITY; index += 1) {
        if(_channelBuffer[index] == pitch) {
            _channelBuffer[index] = -1;
            _channelCountBuffer -= 1;
            return true;
        }
    }

    return false;
}

// Stops playing all notes. Takes effect on the next update
void speakerStopAll() {
    for(uint8_t index = 0; index < _SPEAKER_CHANNEL_CAPACITY; index += 1)
        _channelBuffer[index] = -1;
    _channelCountBuffer = 0;
}


// Pushes the buffered channels
void speakerUpdate() {
    _channels = _channelBuffer;
    _channelCount = _channelCountBuffer;
}

#endif // __SPEAKER_H_
