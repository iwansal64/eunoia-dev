#include <Arduino.h>
#include <VibrationMotor.h>



void VibrationMotor::init() {
  pinMode(VIBRATION_PIN, OUTPUT);
}

void VibrationMotor::vibrate(uint8_t strength) {
      analogWrite(VIBRATION_PIN, strength);
}