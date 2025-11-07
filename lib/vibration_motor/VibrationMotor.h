#include <Arduino.h>

#define VIBRATION_PIN 8             // ADC pin connected to vibration motor signal pin


class VibrationMotor
{
public:
      static void init();
      static void vibrate(uint8_t strength);
};

