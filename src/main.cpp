#include <Arduino.h>

#include "button_matrix/button_matrix.hpp"
#include "display/display.hpp"
#include "speaker/speaker.hpp"

/*
  //Joystick analogue in
  const int JOYY_PIN = A0;
  const int JOYX_PIN = A1;

*/

void setup() {
    /*

    //Set pin directions
    pinMode(OUTL_PIN, OUTPUT);
    pinMode(OUTR_PIN, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);

    pinMode(JOYX_PIN, INPUT);
    pinMode(JOYY_PIN, INPUT);

    */
    ButtonMatrix::initialize();
    Display::initialize();

    //Initialise UART
    Serial.begin(9600);
    Serial.println("Hello World");
}

void loop() {
    Speaker::state.volume = 128;
    Speaker::state.pitch = 256;
    Speaker::state.playing = true;

    static uint32_t next = millis();
    static uint32_t count = 0;

    if (millis() > next) {
        next += 100;

        const uint8_t keys = ButtonMatrix::readColumns();
        Display::clear();
        Display::writeHexadecimal(2, 20, keys);
        Display::update();
    }
}
