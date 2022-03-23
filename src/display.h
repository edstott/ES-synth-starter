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

static U8G2_SSD1305_128X32_NONAME_F_HW_I2C _context(U8G2_R0);

// ----------------------------------------------------------------------------


void displayClear() {
    _context.clearBuffer();
}

void displayInitialize() {
    matrixWrite(_DISPLAY_ADDRESS_RESET, LOW);
    delayMicroseconds(2);
    matrixWrite(_DISPLAY_ADDRESS_RESET, HIGH);

    _context.begin();
    matrixWrite(_DISPLAY_ADDRESS_ENABLE, HIGH);
    _context.setFont(u8g2_font_pxplusibmvga9_mf);

    delayMicroseconds(2);

    _context.clearBuffer();
    _context.sendBuffer();

    Serial.println("display initialized");
}

void displayUpdate() {
    _context.sendBuffer();
}

void displayWriteRadixXY(const uint8_t x, 
        const uint8_t y, 
        const uint8_t value,
        const uint8_t radix) {
    
    _context.setCursor(x, y);
    _context.print(value, radix);
}

void displayWriteXY(const uint8_t x, const uint8_t y, const char *text) {
    _context.drawStr(x, y, text);
}

#endif // __DISPLAY_H_
