#pragma once

#include <Arduino.h>

namespace ButtonMatrix {

enum Pins {
    ROW_0 = D3,
    ROW_1 = D6,
    ROW_2 = D12,

    ROW_ENABLE = A5,

    COLUMN_0 = A2,
    COLUMN_1 = D9,
    COLUMN_2 = A6,
    COLUMN_3 = D1,

    VALUE = D11
};

void initialize();

uint8_t readColumns();

void write(const uint8_t &address, const bool &value);

}; // Namespace ButtonMatrix
