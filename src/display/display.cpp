#include "display.hpp"

namespace Display {

void clear() {
    context.clearBuffer();
}

void initialize() {
    static bool initialized = false;
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

void reset() {
    ButtonMatrix::write(Address::RESET, LOW);
    delayMicroseconds(2);
    ButtonMatrix::write(Address::RESET, HIGH);
}

void update() {
    context.sendBuffer();
}

void write(const uint8_t &column, const uint8_t &row, const char *text) {
    context.drawStr(2, 10, text);
}

void writeHexadecimal(const uint8_t &column,
        const uint8_t &row,
        const uint8_t &value) {

    context.setCursor(column, row);
    context.print(value, HEX);
}

}; // Namespace Display
