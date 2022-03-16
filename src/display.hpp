#include <Arduino.h>
#include <U8g2lib.h>

namespace Display {

enum Address {
    RESET = 4,
    ENABLE = 3,

    HKOW = 5,
    HKOE = 6,
};

static U8G2_SSD1305_128X32_NONAME_F_HW_I2C context(U8G2_R0);

void clear() {
    context.clearBuffer();
}

void initialize() {
    Matrix::write(Address::RESET, LOW);
    delayMicroseconds(2);
    Matrix::write(Address::RESET, HIGH);

    context.begin();
    Matrix::write(Address::ENABLE, HIGH);
    context.setFont(u8g2_font_pxplusibmvga9_mf);
}

void update() {
    context.sendBuffer();
}

void write(const uint8_t &x, const uint8_t &y, const char *text) {
    context.drawStr(x, y, text);
}

void writeHexadecimal(const uint8_t &x,
        const uint8_t &y,
        const uint8_t &value) {

    context.setCursor(x, y);
    context.print(value, HEX);
}

};