#ifndef __DISPLAY_H_
#define __DISPLAY_H_

#include <Arduino.h>
#include <U8g2lib.h>

#include "matrix.h"

// ----------------------------------------------------------------------------

#define _DISPLAY_ADDRESS_RESET  3
#define _DISPLAY_ADDRESS_ENABLE 4

// ----------------------------------------------------------------------------

static U8G2_SSD1305_128X32_NONAME_F_HW_I2C context(U8G2_R0);

// ----------------------------------------------------------------------------

void displayInitialize() {
    matrixWrite(_DISPLAY_ADDRESS_RESET, LOW);
    delayMicroseconds(2);
    matrixWrite(_DISPLAY_ADDRESS_RESET, HIGH);

    context.begin();
    matrixWrite(_DISPLAY_ADDRESS_ENABLE, HIGH);
    context.setFont(u8g2_font_pxplusibmvga9_mf);
}

void displayClear() {
    context.clearBuffer();
}

void displayUpdate() {
    context.sendBuffer();
}

void displayWriteXY(const uint8_t x, const uint8_t y, const char *text) {
    context.drawStr(x, y, text);
}

#endif // __DISPLAY_H_
