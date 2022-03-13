// #include <Arduino.h>
// #include <U8g2lib.h>

#include "button_matrix.hpp"
#include "display.hpp"

/*
  //Audio analogue out
  const int OUTL_PIN = A4;
  const int OUTR_PIN = A3;

  //Joystick analogue in
  const int JOYY_PIN = A0;
  const int JOYX_PIN = A1;

*/

void setup() {
  // put your setup code here, to run once:

    /*

  //Set pin directions
  pinMode(OUTL_PIN, OUTPUT);
  pinMode(OUTR_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(JOYX_PIN, INPUT);
  pinMode(JOYY_PIN, INPUT);

  */
    ButtonMatrix::initialize();
    Display::initialize();


    //Initialise UART
    Serial.begin(9600);
    Serial.println("Hello World");
}

void loop() {
        
    // put your main code here, to run repeatedly:
    static uint32_t next = millis();
    static uint32_t count = 0;

    if (millis() > next) {
        next += 100;

        uint8_t keys = ButtonMatrix::readColumns();
        Display::context.clearBuffer();
        Display::context.setCursor(2,20);
        Display::context.print(keys, HEX);
        Display::context.sendBuffer();
    }
}