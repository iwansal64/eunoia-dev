// ------------- Libraries
#include <Arduino.h>
#include <PulseSensor.h>
#include <VibrationMotor.h>
#include <BluetoothManager.h>

// ------------- Defines
#define UPDATE_DATA_INTERVAL 5000    // Update data through bluetooth every N miliseconds
#define BPM_OUTLIER_MAX 150          // Maximal BPM that is count as valid
#define BPM_OUTLIER_MIN 40           // Minimal BPM that is counts as valid
#define BPM_MICROSLEEP_DOWN_SPIKE 10 // Microsleep is detected when there's a down spike from previous BPM

#define MICROSLEEP_VIBRATE_COUNT 5  // How many vibrations when detected microsleep
#define VIBRATION_LOW_TIME 500      // Wait to vibrate again after what miliseconds
#define VIBRATION_HIGH_TIME 1000    // Vibrate for what miliseconds
#define VIBRATION_POWER_PERCENT 80  // How much the power of vibration (max: 100)

// ------------- Variables
// Vibration Motor Related Variables
unsigned long last_vibrate = 0U;
bool vibrate_state = false;

// Communication Related Variables
unsigned long last_update_bpm = 0U;
uint8_t last_broadcasted_bpm = 0;
uint8_t last_bpm = 0;

// Microsleep Related Variables
bool microsleep_detected = false;
uint8_t microsleep_timing = 0;

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

  Serial.println("[MAIN] Starting..");
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
  uint64_t current_time = millis();

  int current_bpm = PulseSensor::BPM;

  // Check for outliers
  if (current_bpm > BPM_OUTLIER_MAX || current_bpm < BPM_OUTLIER_MIN) {
    VibrationMotor::vibrate(0);
    yield();
    return;
  }

  // If the BPM spiked down from last BPM
  if (!microsleep_detected && current_bpm < (last_bpm - BPM_MICROSLEEP_DOWN_SPIKE))
  {
    microsleep_detected = true;
    Serial.println("[MAIN] Microsleep Detected!");
  }

  // If microsleep detected
  if(microsleep_detected) {
    // Vibrate for some interval
    if (current_time - last_vibrate > (vibrate_state ? VIBRATION_HIGH_TIME : VIBRATION_LOW_TIME))
    {
      vibrate_state = !vibrate_state;                                                                       // Alternate between vibrate and not
      VibrationMotor::vibrate(vibrate_state ? ((int)(round(255 * (VIBRATION_POWER_PERCENT / 100.0)))) : 0); // Vibrating (or not)
      last_vibrate = current_time;                                                                          // Update current time

      if(!vibrate_state) {
        microsleep_timing--;
      }

      if(microsleep_timing <= 0) {
        microsleep_detected = false;
        Serial.println("[MAIN] Microsleep Retrieved!");
        VibrationMotor::vibrate(0);
      }
    }
  }

  last_bpm = current_bpm;

  yield();
}

void update_hr_data()
{
  uint64_t current_time = millis();

  // Update Heart Rate data every interval
  if(current_time - last_update_bpm > UPDATE_DATA_INTERVAL) {
    int current_bpm = PulseSensor::BPM;

    // Check for outliers
    if(current_bpm > BPM_OUTLIER_MAX || current_bpm < BPM_OUTLIER_MIN) {
      yield();
      return;
    }
    
    // If current BPM is different from the last value
    if(last_broadcasted_bpm != PulseSensor::BPM) {
      BluetoothManager::update_bpm(PulseSensor::BPM);
      last_broadcasted_bpm = PulseSensor::BPM;
      last_update_bpm = current_time; // Update last update HR time
    }
  }
  
  yield();
}
