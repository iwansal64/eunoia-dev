#include <Arduino.h>
#include <VibrationMotor.h>

void VibrationMotor::init()
{
      Serial.println("[Vibrator] Initializing Vibration Motor");
      pinMode(VIBRATION_PIN, OUTPUT);
}

void VibrationMotor::vibrate(uint8_t strength)
{
      analogWrite(VIBRATION_PIN, strength);
}