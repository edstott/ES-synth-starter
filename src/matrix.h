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
#define KNOB_0_MAX 7    // Octaves
#define KNOB_0_MIN 1    // Octaves
#define KNOB_1_MAX 2    // Waveforms
#define KNOB_1_MIN 0    // Waveforms
#define KNOB_2_MAX 127
#define KNOB_2_MIN 0
#define KNOB_3_MAX 100  // Volume
#define KNOB_3_MIN 0    // Volume

#define JOYSTICK_S   _MATRIX_ENCODE_INDEX(5, 2)
#define EAST_DETECT  _MATRIX_ENCODE_INDEX(6, 3)
#define WEST_DETECT  _MATRIX_ENCODE_INDEX(5, 3)

// ----------------------------------------------------------------------------

static const uint8_t encoder_lut[16] = {0x0, 0x0, 0x0, 0x3,
                                        0x2, 0x0, 0x3, 0x1,
                                        0x1, 0x3, 0x0, 0x2,
                                        0x3, 0x0, 0x0, 0x0};

static const uint8_t encoder_max[] = {KNOB_0_MAX,
                                      KNOB_1_MAX,
                                      KNOB_2_MAX,
                                      KNOB_3_MAX};

static const uint8_t encoder_min[] = {KNOB_0_MIN,
                                      KNOB_1_MIN,
                                      KNOB_2_MIN,
                                      KNOB_3_MIN};
uint8_t KNOB[4];
bool KNOB_EXTRA[4];

uint8_t encoderAngle[4];

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
    Serial.println("matrix initialized");
    encoderAngle[3] = 50;
}

void encoderHandle(uint8_t *rows){
    for (int i = 0; i < 4; i++){
        if (encoder_lut[KNOB[i]] == 0x03) KNOB_EXTRA[i] = false;
        else KNOB_EXTRA[i] = false;
    }
    // Load current encoder values into encoder chars
    KNOB[0] = 0x0F & ((KNOB[0] << 2) | ((rows[2] & 0x0C)) >> 2);
    KNOB[1] = 0x0F & ((KNOB[1] << 2) | (rows[2] & 0x03));
    KNOB[2] = 0x0F & ((KNOB[2] << 2) | ((rows[1] & 0xC0)) >> 6);
    KNOB[3] = 0x0F & ((KNOB[3] << 2) | ((rows[1] & 0x30)) >> 4);

    for (int i = 0; i < 4; i++){
        if (encoder_lut[KNOB[i]] == 1){
            if (KNOB_EXTRA[i] && encoderAngle[i] <= encoder_max[i]-2) encoderAngle[i] += 2;
            else if (encoderAngle[i] <= encoder_max[i]-1) encoderAngle[i] += 1;
        }else if (encoder_lut[KNOB[i]] == 2){
            if (KNOB_EXTRA[i] && encoderAngle[i] >= encoder_min[i]+2) encoderAngle[i] -= 2;
            else if (encoderAngle[i] >= encoder_min[i]+1) encoderAngle[i] -= 1;
        }
    }
}

void matrixRead(uint8_t *rows) {

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

        if(rowIndex % 2 == 0)
            rows[rowIndex / 2] = ~result &0xF;
        else
            rows[rowIndex / 2] |= (~result & 0xF) << 4;
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
