#include <Arduino.h>

#include "matrix.hpp"
#include "display.hpp"
#include "speaker.hpp"

void setup() {

    Speaker::initialize();
    Matrix::initialize();
    Display::initialize();

    //Initialise UART
    Serial.begin(9600);
    Serial.println("device online");
}

void loop() {
    static uint32_t next = millis();

    if (millis() > next) {
        next += 100;

        Matrix::update();

        char buffer[12] = {'\0'};
        for(int row = 0; row < 3; row += 1) {
            for(int column = 0; column < 4; column += 1) {
                const char value = (Matrix::values[row] >> column) & 0x1 ? '@' : '.';
                strncat(buffer, &value, 1);
            }
        }

        Display::clear();
        Display::write(2, 10, buffer);
        Display::update();
    }
}
