#include <Arduino.h>

#define VIBRATION_PIN 18            // ADC pin connected to vibration motor signal pin
#define VIBRATION_LOW_TIME 500      // Wait to vibrate again after what miliseconds
#define VIBRATION_HIGH_TIME 1000    // Vibrate for what miliseconds
#define VIBRATION_POWER_PERCENT 80  // How much the power of vibration (max: 100)


class VibrationMotor
{
public:
      static void init();
      static void vibrate(uint8_t strength);
};

