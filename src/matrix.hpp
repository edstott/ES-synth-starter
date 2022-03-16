#include <Arduino.h>

#define _INDEX_ENCODE(row, column) ((row << 4) | column)

#define _INDEX_ROW(index)    (index >> 4)
#define _INDEX_COLUMN(index) (index & 0xF)

namespace Matrix {

enum Pins {
    ROW_0 = D3,
    ROW_1 = D6,
    ROW_2 = D12,

    ENABLE = A5,

    COLUMN_0 = A2,
    COLUMN_1 = D9,
    COLUMN_2 = A6,
    COLUMN_3 = D1,

    VALUE = D11,
};

enum Input {

    // Piano keys
    KEY_C  = _INDEX_ENCODE(0, 0),
    KEY_CS = _INDEX_ENCODE(0, 1),
    KEY_D  = _INDEX_ENCODE(0, 2),
    KEY_DS = _INDEX_ENCODE(0, 3),
    KEY_E  = _INDEX_ENCODE(1, 0),
    KEY_F  = _INDEX_ENCODE(1, 1),
    KEY_FS = _INDEX_ENCODE(1, 2),
    KEY_G  = _INDEX_ENCODE(1, 3),
    KEY_GS = _INDEX_ENCODE(2, 0),
    KEY_A  = _INDEX_ENCODE(2, 1),
    KEY_AS = _INDEX_ENCODE(2, 2),
    KEY_B  = _INDEX_ENCODE(2, 3),

    // Rotary encoders
    KNOB_3_A = _INDEX_ENCODE(3, 0),
    KNOB_3_B = _INDEX_ENCODE(3, 1),
    KNOB_3_S = _INDEX_ENCODE(5, 1),

    KNOB_2_A = _INDEX_ENCODE(3, 2),
    KNOB_2_B = _INDEX_ENCODE(3, 3),
    KNOB_2_S = _INDEX_ENCODE(5, 0),

    KNOB_1_A = _INDEX_ENCODE(4, 0),
    KNOB_1_B = _INDEX_ENCODE(4, 1),
    KNOB_1_S = _INDEX_ENCODE(6, 1),

    KNOB_0_A = _INDEX_ENCODE(4, 2),
    KNOB_0_B = _INDEX_ENCODE(4, 3),
    KNOB_0_S = _INDEX_ENCODE(6, 0),

    // Joystick
    JOYSTICK_S    = _INDEX_ENCODE(5, 2),
    JOYSTICK_EAST = _INDEX_ENCODE(6, 3),
    JOYSTICK_WEST = _INDEX_ENCODE(5, 3),
};

static uint8_t values[8];

void initialize() {
    pinMode(Pins::ROW_0, OUTPUT);
    pinMode(Pins::ROW_1, OUTPUT);
    pinMode(Pins::ROW_2, OUTPUT);
    pinMode(Pins::ENABLE, OUTPUT);

    pinMode(Pins::COLUMN_0, INPUT);
    pinMode(Pins::COLUMN_1, INPUT);
    pinMode(Pins::COLUMN_2, INPUT);
    pinMode(Pins::COLUMN_3, INPUT);
    
    pinMode(Pins::VALUE, OUTPUT);
}

/* Reads the values of a row of the matrix
*/
uint8_t readRow(const uint8_t &rowIndex) {

    digitalWrite(Pins::ENABLE, LOW);

    digitalWrite(Pins::ROW_0, rowIndex & 0x1);
    digitalWrite(Pins::ROW_1, rowIndex & 0x2);
    digitalWrite(Pins::ROW_2, rowIndex & 0x4);

    digitalWrite(Pins::ENABLE, HIGH);

    delayMicroseconds(3);

    const uint8_t result = digitalRead(Pins::COLUMN_0)
            | digitalRead(Pins::COLUMN_1) << 1
            | digitalRead(Pins::COLUMN_2) << 2
            | digitalRead(Pins::COLUMN_3) << 3;
    
    return ~result & 0xF;
}

/* Refreshes the cached value table
*/
void update() {
    for(uint8_t rowIndex = 0; rowIndex < 8; rowIndex += 1)
        values[rowIndex] = readRow(rowIndex);
}

/* Reads the state of a given input
*/
bool readInput(const Input &inputIndex) {
    const uint8_t rowValue = values[_INDEX_ROW(inputIndex)];
    return (rowValue >> _INDEX_COLUMN(inputIndex)) & 0x1;
}

/* Writes a value to a given index
*/
void write(const uint8_t &address, const bool &value) {
    digitalWrite(Pins::ENABLE, LOW);

    digitalWrite(Pins::ROW_0, address & 0x01);
    digitalWrite(Pins::ROW_1, address & 0x02);
    digitalWrite(Pins::ROW_2, address & 0x04);

    digitalWrite(Pins::VALUE, value);

    digitalWrite(Pins::ENABLE, HIGH);
    delayMicroseconds(2);
    digitalWrite(Pins::ENABLE, LOW);
}

};
