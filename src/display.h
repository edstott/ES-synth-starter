#ifndef __DISPLAY_H_
#define __DISPLAY_H_

#include <Arduino.h>
#include <U8g2lib.h>

#include "matrix.h"

// ----------------------------------------------------------------------------

#define _DISPLAY_ADDRESS_RESET  4
#define _DISPLAY_ADDRESS_ENABLE 3

// ----------------------------------------------------------------------------

#define RADIX_BINARY      2
#define RADIX_OCTAL       8
#define RADIX_HEXADECIMAL 16
#define RADIX_DECIMAL     10

// ----------------------------------------------------------------------------

static U8G2_SSD1305_128X32_NONAME_F_HW_I2C context(U8G2_R0);

// ----------------------------------------------------------------------------


void displayClear() {
    context.clearBuffer();
}

void displayInitialize() {
    matrixWrite(_DISPLAY_ADDRESS_RESET, LOW);
    delayMicroseconds(2);
    matrixWrite(_DISPLAY_ADDRESS_RESET, HIGH);

    context.begin();
    matrixWrite(_DISPLAY_ADDRESS_ENABLE, HIGH);
    context.setFont(u8g2_font_pxplusibmvga9_mf);

    delayMicroseconds(2);

    context.clearBuffer();
    context.sendBuffer();

    Serial.println("display initialized");
}

void displayUpdate() {
    context.sendBuffer();
}

void displayWriteRadixXY(const uint8_t x, 
        const uint8_t y, 
        const uint8_t value,
        const uint8_t radix) {
    
    context.setCursor(x, y);
    context.print(value, radix);
}

void displayWriteXY(const uint8_t x, const uint8_t y, const char *text) {
    context.drawStr(x, y, text);
}

#endif // __DISPLAY_H_
