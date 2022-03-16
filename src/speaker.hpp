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

struct Note {

public:

    Key key;

    int8_t octave;

    Note() : key(Key::C), octave(4) {}

    Note(const Key &key, const int8_t &octave) {
        this->key = key;
        this->octave = octave;
    }

};

static const uint16_t updatePeriod = 22000;

static uint16_t pitches[6];

Waveform waveform = Waveform::SAWTOOTH;

/* Generates a sawtooth waveform

Arguments
---------
pitch: const uint32_t &
    the pitch (frequency) of the waveform
time: const uint32_t &
    the current time since initialization in microseconds

Returns
-------
amplitude: uint8_t
    the corresponding output amplitude
*/
uint8_t generateSawtooth(const uint32_t &pitch, const uint32_t &time) {
	const uint32_t period = 1e6 / pitch;
	const uint32_t phase = time % period;
	return (phase << 7) / period + 127;
}

/* Generates a square waveform

Arguments
---------
pitch: const uint32_t &
    the pitch (frequency) of the waveform
time: const uint32_t &
    the current time since initialization in microseconds

Returns
-------
amplitude: uint8_t
    the corresponding output amplitude
*/
uint8_t generateSquare(const uint32_t &pitch, const uint32_t &time) {
	const uint32_t period = 1e6 / pitch;
	const uint32_t phase = time % period;
    return (phase < period / 2) ? 0 : 255;
}

/* Generates a triangle waveform

Arguments
---------
pitch: const uint32_t &
    the pitch (frequency) of the waveform
time: const uint32_t &
    the current time since initialization in microseconds

Returns
-------
amplitude: uint8_t
    the corresponding output amplitude
*/
uint8_t generateTriangle(const uint32_t &pitch, const uint32_t &time) {
	const uint32_t period = 1e6 / pitch;
	const uint32_t phase = time % period;
	uint32_t result = (phase << 9) / period;
    return (phase > (period / 2)) ? 512 - result : result;
}

/* Scales a signal to a given volume range

Arguments
---------
value: const uint8_t &
    the value whose volume to scale, presuming it's in the range (0, 256]
volume: const uint8_t &
    the volume level to which to scale, with 255 representing no change, and
    0 being silent

Returns
-------
amplitude: uint8_t
    the value scaled to the given volume
*/
uint8_t scaleVolume(const uint8_t &value, const uint8_t &volume) {
    const int16_t delta = value - 127;
    const int16_t adjust = (delta * volume) >> 8;
    return 127 + adjust;
}

/* Mixes a number of signal channels together

Uses the headroom paradigm to ensure all the channels (when summed) fit in the
maximum range. Basically a summed average

Arguments
---------
channels: const uint8_t *
    pointer to an array of amplitude values
channelCount: const uint8_t &
    the number of channels in the array

Returns
-------
amplitude: uint8_t
    the amplitude of the resulting mixed signal
*/
uint8_t mix(const uint8_t *channels, const uint8_t &channelCount) {
    uint16_t result = 0;
    for(uint8_t index = 0; index < channelCount; index += 1)
        result += channels[index];
    return result / channelCount;
}

/* Gets the pitch of a given key

Uses some ~fancy~ math to evaluate the pitch of the key, relative to A4=440Hz.
Becomes less precise, the further away the given key is from A4.

Uses fixed-point arithmetic with a "precision" of 3 decimal places.

Arguments
---------
key: const Key &
    the key of the note
octave: const uint8_t
    the octave the note is in

Returns
-------
pitch: uint16_t
    the pitch of the note, in Hertz
*/
uint16_t keyPitch(const Key &key, const uint8_t octave = 4) {
    uint32_t result = 440;

    const int8_t octaveDelta = octave - 4;
    int16_t steps = key + (octaveDelta * 11);

    const bool positive = steps > 0;
    steps = (steps > 0) ? steps : -steps;

    result <<= 3;
    for(uint8_t step = 0; step < steps; step += 1) {
        result *= positive ? 1059463 : 1e6;
        result /= positive ? 1e6 : 1059463;
    }
    result >>= 3;

    return result;
}

/* Audio ISR

Interrupt service routine which mixes the channels of each note being played,
scales the result to the current volume level, and writes the result to the
on-board speaker
*/
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

/* Initializes speaker

Configures ports and registers ISR
*/
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
