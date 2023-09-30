#include "pinout.h"

pinout *pinout::_instance;

// [static]
pinout &pinout::instance() {
    if (!_instance) {
        _instance = new pinout();
    }
    return *_instance;
}

pinout::pinout() {
}

pinout::~pinout() {
}

void pinout::setup() {
  //Setup pins to be ins or outs. 
  pinMode(CE, OUTPUT);
  pinMode(RFM95_CS,OUTPUT);
  pinMode(RFM95_DIO0,INPUT_PULLUP);
  pinMode(RFM95_RST, OUTPUT);
  pinMode(INT_PIN, INPUT_PULLUP);
  pinMode(Enable, OUTPUT);
  
  //Establish a random seeq based on the interrupt pin
  randomSeed(analogRead(INT_PIN));
}

void pinout::loop() {
    // Put your code to run during the application thread loop here
}
