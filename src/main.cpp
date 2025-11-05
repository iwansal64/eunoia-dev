// ------------- Libraries
#include <Arduino.h>
#include <PulseSensor.h>
#include <VibrationMotor.h>


// ------------- Variables
unsigned long last_vibrate = 0;
bool vibrate_state = false;

// ------------- Function Definitions
void check_for_microsleep(); // Used for detecting microsleep


// ------------- Setup
void setup() {
  Serial.begin(9600);

  PulseSensor::init();
  VibrationMotor::init();

  pinMode(LED_BUILTIN, OUTPUT);
}

// ------------- Loop
void loop() {
  check_for_microsleep();
}


// ------------- Functions Implementations
void check_for_microsleep() {
  unsigned long current_time = millis();

  if(PulseSensor::BPM < 70) {
    if(current_time - last_vibrate > (vibrate_state ? VIBRATION_HIGH_TIME : VIBRATION_LOW_TIME)) {             // Vibrate for each several seconds
      vibrate_state = !vibrate_state;                                                                          // Alternate between vibrate and not
      VibrationMotor::vibrate(vibrate_state ? ((int) (round(255 * (VIBRATION_POWER_PERCENT / 100.0)))) : 0);   // Vibrating (or not)
      last_vibrate = current_time;                                                                             // Update current time
    }
  }
  else {
    VibrationMotor::vibrate(0);
  }
}
