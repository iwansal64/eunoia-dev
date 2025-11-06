// ------------- Libraries
#include <Arduino.h>
#include <PulseSensor.h>
#include <VibrationMotor.h>
#include <BluetoothManager.h>

// ------------- Defines
#define UPDATE_DATA_INTERVAL 5000

// ------------- Variables
// Vibration Motor Variables
unsigned long last_vibrate = 0U;
bool vibrate_state = false;

// Communication Variables
unsigned long last_update_bpm = 0U;
uint8_t last_bpm = 0;

// ------------- Function Definitions
void check_for_microsleep(); // Used for detecting microsleep
void update_hr_data();

// ------------- Setup
void setup()
{
  Serial.begin(9600);
  Serial.println();

  PulseSensor::init();
  VibrationMotor::init();
  BluetoothManager::init();

  pinMode(LED_BUILTIN, OUTPUT);
}

// ------------- Loop
void loop()
{
  PulseSensor::loop();      // Update the value of pulse sensor each loop
  update_hr_data();         // Update Heartrate data from pulse sensor
  check_for_microsleep();   // Check for microsleep
}

// ------------- Functions Implementations
void check_for_microsleep()
{
  unsigned long current_time = millis();

  // Detects anomalities in BPM value
  if (PulseSensor::BPM < 10) {
    return;
  }

  // If the BPM is less than 70
  if (PulseSensor::BPM < 70)
  {
    // Vibrate for some interval
    if (current_time - last_vibrate > (vibrate_state ? VIBRATION_HIGH_TIME : VIBRATION_LOW_TIME))
    {
      vibrate_state = !vibrate_state;                                                                       // Alternate between vibrate and not
      VibrationMotor::vibrate(vibrate_state ? ((int)(round(255 * (VIBRATION_POWER_PERCENT / 100.0)))) : 0); // Vibrating (or not)
      last_vibrate = current_time;                                                                          // Update current time
    }
  }
  else
  {
    // If the BPM is normal, don't vibrate
    VibrationMotor::vibrate(0);
  }
}

void update_hr_data()
{
  unsigned long current_time = millis();

  // Update Heart Rate data every interval
  if(current_time - last_update_bpm > UPDATE_DATA_INTERVAL) {
    if(last_bpm != PulseSensor::BPM) {
      BluetoothManager::update_bpm(PulseSensor::BPM);
      last_update_bpm = current_time; // Update last update HR time
    }
  }
}
