#ifndef __MATRIX_H_
#define __MATRIX_H_

#include <Arduino.h>

// ----------------------------------------------------------------------------

#define _MATRIX_PIN_COLUMN_0 A2
#define _MATRIX_PIN_COLUMN_1 D9
#define _MATRIX_PIN_COLUMN_2 A6
#define _MATRIX_PIN_COLUMN_3 D1

#define _MATRIX_PIN_ROW_0    D3
#define _MATRIX_PIN_ROW_1    D6
#define _MATRIX_PIN_ROW_2    D12

#define _MATRIX_PIN_ENABLE   A5
#define _MATRIX_PIN_VALUE    D11

// ----------------------------------------------------------------------------

#define _MATRIX_ENCODE_INDEX(row, column) \
        (((row / 2) << 4) | ((row % 2) ? column + 4 : column))

#define KEY_C  _MATRIX_ENCODE_INDEX(0, 0)
#define KEY_CS _MATRIX_ENCODE_INDEX(0, 1)
#define KEY_D  _MATRIX_ENCODE_INDEX(0, 2)
#define KEY_DS _MATRIX_ENCODE_INDEX(0, 3)
#define KEY_E  _MATRIX_ENCODE_INDEX(1, 0)
#define KEY_F  _MATRIX_ENCODE_INDEX(1, 1)
#define KEY_FS _MATRIX_ENCODE_INDEX(1, 2)
#define KEY_G  _MATRIX_ENCODE_INDEX(1, 3)
#define KEY_GS _MATRIX_ENCODE_INDEX(2, 0)
#define KEY_A  _MATRIX_ENCODE_INDEX(2, 1)
#define KEY_AS _MATRIX_ENCODE_INDEX(2, 2)
#define KEY_B  _MATRIX_ENCODE_INDEX(2, 3)

#define KNOB_0_A _MATRIX_ENCODE_INDEX(4, 2)
#define KNOB_0_B _MATRIX_ENCODE_INDEX(4, 3)
#define KNOB_0_S _MATRIX_ENCODE_INDEX(6, 0)
#define KNOB_1_A _MATRIX_ENCODE_INDEX(4, 0)
#define KNOB_1_B _MATRIX_ENCODE_INDEX(4, 1)
#define KNOB_1_S _MATRIX_ENCODE_INDEX(6, 1)
#define KNOB_2_A _MATRIX_ENCODE_INDEX(3, 2)
#define KNOB_2_B _MATRIX_ENCODE_INDEX(3, 3)
#define KNOB_2_S _MATRIX_ENCODE_INDEX(5, 0)
#define KNOB_3_A _MATRIX_ENCODE_INDEX(3, 0)
#define KNOB_3_B _MATRIX_ENCODE_INDEX(3, 1)
#define KNOB_3_S _MATRIX_ENCODE_INDEX(5, 1)

#define JOYSTICK_S    _MATRIX_ENCODE_INDEX(5, 2)
#define JOYSTICK_EAST _MATRIX_ENCODE_INDEX(6, 3)
#define JOYSTICK_WEST _MATRIX_ENCODE_INDEX(5, 3)

// ----------------------------------------------------------------------------

void matrixInitialize() {
    pinMode(_MATRIX_PIN_ROW_0, OUTPUT);
    pinMode(_MATRIX_PIN_ROW_1, OUTPUT);
    pinMode(_MATRIX_PIN_ROW_2, OUTPUT);
    pinMode(_MATRIX_PIN_ENABLE, OUTPUT);

    pinMode(_MATRIX_PIN_COLUMN_0, INPUT);
    pinMode(_MATRIX_PIN_COLUMN_1, INPUT);
    pinMode(_MATRIX_PIN_COLUMN_2, INPUT);
    pinMode(_MATRIX_PIN_COLUMN_3, INPUT);

    pinMode(_MATRIX_PIN_VALUE, OUTPUT);
}

void matrixRead(uint8_t *rows) {

    uint8_t offset = 0;
    for(uint8_t rowIndex = 0; rowIndex < 8; rowIndex += 1) {
        digitalWrite(_MATRIX_PIN_ENABLE, LOW);

        digitalWrite(_MATRIX_PIN_ROW_0, rowIndex & 0x1);
        digitalWrite(_MATRIX_PIN_ROW_1, rowIndex & 0x2);
        digitalWrite(_MATRIX_PIN_ROW_2, rowIndex & 0x4);

        digitalWrite(_MATRIX_PIN_ENABLE, HIGH);

        delayMicroseconds(3);

        const uint8_t result = digitalRead(_MATRIX_PIN_COLUMN_0)
                | digitalRead(_MATRIX_PIN_COLUMN_1) << 1
                | digitalRead(_MATRIX_PIN_COLUMN_2) << 2
                | digitalRead(_MATRIX_PIN_COLUMN_3) << 3;

        rows[rowIndex / 2] = (~result & 0xF) << (4 * offset);
        offset = !offset;
    }
}

uint8_t matrixKeyPressed(uint8_t *rows, const uint8_t address) {
    const uint8_t row = address >> 4;
    const uint8_t column = address & 0xF;
    return (rows[row] >> column) & 0x1;
}

void matrixWrite(const uint8_t address, const uint8_t value) {
    digitalWrite(_MATRIX_PIN_ENABLE, LOW);

    digitalWrite(_MATRIX_PIN_ROW_0, address & 0x01);
    digitalWrite(_MATRIX_PIN_ROW_1, address & 0x02);
    digitalWrite(_MATRIX_PIN_ROW_2, address & 0x04);

    digitalWrite(_MATRIX_PIN_VALUE, value);

    digitalWrite(_MATRIX_PIN_ENABLE, HIGH);
    delayMicroseconds(2);
    digitalWrite(_MATRIX_PIN_ENABLE, LOW);
}

#endif // __MATRIX_H_
