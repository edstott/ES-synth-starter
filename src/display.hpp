#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

#include "button_matrix.hpp"

namespace Display {

enum Address {
    RESET = 4,
    ENABLE = 3,

    HKOW = 5,
    HKOE = 6,
};

static U8G2_SSD1305_128X32_NONAME_F_HW_I2C context(U8G2_R0);

static bool initialized = false;

void reset() {
    ButtonMatrix::write(Address::RESET, LOW);
    delayMicroseconds(2);
    ButtonMatrix::write(Address::RESET, HIGH);
}

void initialize() {
    if(initialized)
        return;
    
    ButtonMatrix::initialize();

    reset();

    ButtonMatrix::write(Address::RESET, LOW);
    delayMicroseconds(2);
    ButtonMatrix::write(Address::RESET, HIGH);

    context.begin();
    ButtonMatrix::write(Address::ENABLE, HIGH);
    context.setFont(u8g2_font_ncenB08_tr);

    initialized = true;
}

void clear() {
    context.clearBuffer();
}

void write(const uint8_t &column, const uint8_t &row, const char *text) {
    clear();
    context.drawStr(2, 10, text);
}

void update() {
    context.sendBuffer();
}

};