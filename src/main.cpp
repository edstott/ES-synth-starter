#include <Arduino.h>

#include "matrix.h"
#include "display.h"
#include "speaker.h"

void setup() {
    Serial.begin(9600);
    Serial.println("device started");

    matrixInitialize();
    displayInitialize();
    speakerInitialize();

    speakerSetVolume(64);
    speakerSetWaveform(WAVEFORM_SAWTOOTH);
}

int8_t keyNote(const uint8_t key) {
    const uint8_t row = key >> 4;
    const uint8_t column = key & 0xF;
    const uint8_t index = row * 8 + column;
    return index - 9;
}

void loop() {
    static uint32_t next = millis();
    static uint32_t count = 0;

    static const uint8_t keyCount = 12;
    static const uint8_t pianoKeys[12] = {
        KEY_C,
        KEY_CS,
        KEY_D,
        KEY_DS,
        KEY_E,
        KEY_F,
        KEY_FS,
        KEY_G,
        KEY_GS,
        KEY_A,
        KEY_AS,
        KEY_B
    };

    uint8_t scanKeys[4];
    if(millis() > next) {
        next += 100;

        matrixRead(scanKeys);

        displayClear();
        displayWriteRadixXY(2, 10, scanKeys[1], RADIX_BINARY);
        displayUpdate();

        speakerStop();
        for(uint8_t index = 0; index < keyCount; index += 1) {
            const uint8_t key = pianoKeys[index];

            if(matrixKeyPressed(scanKeys, key)) {
                const int8_t value = keyNote(key);
                speakerPlayNote(createNote(value, 4));
            }
        }
        speakerUpdate();
    }
}
