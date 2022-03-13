#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

#include "../button_matrix/button_matrix.hpp"

namespace Display {

enum Address {
    RESET = 4,
    ENABLE = 3,

    HKOW = 5,
    HKOE = 6,
};

static U8G2_SSD1305_128X32_NONAME_F_HW_I2C context(U8G2_R0);

void clear();

void initialize();

void reset();

void update();

void write(const uint8_t &column, const uint8_t &row, const char *text);

void writeHexadecimal(const uint8_t &column,
        const uint8_t &row,
        const uint8_t &value);

}; // Namespace Display
