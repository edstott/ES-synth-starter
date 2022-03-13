#include "button_matrix.hpp"

namespace ButtonMatrix {

void initialize() {
    static bool initialized = false;
    if(initialized)
        return;

    pinMode(Pins::ROW_0, OUTPUT);
    pinMode(Pins::ROW_1, OUTPUT);
    pinMode(Pins::ROW_2, OUTPUT);

    pinMode(Pins::ROW_ENABLE, OUTPUT);

    pinMode(Pins::COLUMN_0, INPUT);
    pinMode(Pins::COLUMN_1, INPUT);
    pinMode(Pins::COLUMN_2, INPUT);
    pinMode(Pins::COLUMN_3, INPUT);

    pinMode(Pins::VALUE, OUTPUT);

    initialized = true;
}

uint8_t readColumns() {
    digitalWrite(ROW_0, LOW);
    digitalWrite(ROW_1, LOW);
    digitalWrite(ROW_2, LOW);

    digitalWrite(ROW_ENABLE, HIGH);

    return digitalRead(COLUMN_0)
            | digitalRead(COLUMN_1) << 1
            | digitalRead(COLUMN_2) << 2
            | digitalRead(COLUMN_3) << 3;
}

void write(const uint8_t &address, const bool &value) {
    digitalWrite(Pins::ROW_ENABLE, LOW);

    digitalWrite(ROW_0, address & 0x01);
    digitalWrite(ROW_1, address & 0x02);
    digitalWrite(ROW_2, address & 0x04);

    digitalWrite(VALUE, value);

    digitalWrite(ROW_ENABLE, HIGH);
    delayMicroseconds(2);
    digitalWrite(ROW_ENABLE, LOW);
}

}; // Namespace ButtonMatrix
