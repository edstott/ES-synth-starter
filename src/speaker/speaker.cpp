#include "speaker.hpp"

namespace Speaker {

static uint8_t generateSawtooth(const float &time, 
        const uint16_t &pitch, 
        const uint16_t &volume) {
        
    const double frequency = pitch * time;
    return volume * (frequency - floor(frequency));
}

static uint8_t generateSine(const float &time, 
        const uint16_t &pitch, 
        const uint16_t &volume) {
        
    return volume * 0.5 * (sin(2.0 * 3.14159 * pitch * time) + 1);
}

static uint8_t generateSquare(const float &time, 
        const uint16_t &pitch, 
        const uint16_t &volume) {
        
    const double frequency = pitch * time;
    const double phase = frequency - floor(frequency);
    return volume * ((phase < 0.5) ? 0 : 1);
}

static uint8_t generateTriangle(const float &time,
        const uint16_t &pitch, 
        const uint16_t &volume) {
        
    const double frequency = pitch * time;
    return volume * 2 * abs(frequency - round(frequency));
}

static void updateRoutine() {

    // Update the time accumulator
    static double time = 0;
    time += 1 / updateFrequency;

    // Zero the output if not playing
    if(state.playing == false)
        analogWrite(Pins::RIGHT, 0);
    
    // Otherwise, calculate the new output value
    float value = 0;
    if(state.waveform == Waveform::SAWTOOTH)
        value = generateSawtooth(time, state.pitch, state.volume);
    else if(state.waveform == Waveform::SAWTOOTH)
        value = generateSawtooth(time, state.pitch, state.volume);
    else if(state.waveform == Waveform::SAWTOOTH)
        value = generateSawtooth(time, state.pitch, state.volume);
    else if(state.waveform == Waveform::SAWTOOTH)
        value = generateSawtooth(time, state.pitch, state.volume);

    analogWrite(Pins::RIGHT, value * state.volume);
}

void initialize() {
    static bool initialized = false;
    if(initialized)
        return;

    // Set the output pin modes
    pinMode(Pins::LEFT, OUTPUT);
    pinMode(Pins::RIGHT, OUTPUT);

    // Create an ISR to update the speaker value
    TIM_TypeDef *timer = TIM1;
    HardwareTimer *timerRoutine = new HardwareTimer(timer);
    timerRoutine->setOverflow(updateFrequency, HERTZ_FORMAT);
    timerRoutine->attachInterrupt(updateRoutine);
    timerRoutine->resume();

    initialized = true;
}

}; // Namespace Speaker