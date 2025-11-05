#include <Arduino.h>
#include <VibrationMotor.h>

void VibrationMotor::init()
{
      Serial.println("[Vibrator] Initializing Vibration Motor");
      pinMode(VIBRATION_PIN, OUTPUT);
}

void VibrationMotor::vibrate(uint8_t strength)
{
      Serial.printf("[Vibrator] Vibrating at this %s Strength\n", String((strength / 255.0f) * 100.0f, (unsigned int)2U));
      analogWrite(VIBRATION_PIN, strength);
}