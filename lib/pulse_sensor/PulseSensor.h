#include <Arduino.h>

#define PULSE_SENSOR_PIN 34            // ADC pin connected to pulse sensor signal
#define PULSE_ANALOG_SCOUNT 5          // The amount of samples for pulse sensor
#define PULSE_ANALOG_THRESHOLD 1900    // The treshold of analog read which we can sure that the sensor is detecting heart beat (Tune this depending on sensor/noise)
#define PULSE_ANALOG_OUTLIER_MIN 1000  // The minimum value of analog read which we sure its a valid value (Tune this depending on sensor/noise)
#define PULSE_ANALOG_OUTLIER_MAX 2200  // The maximum value of analog read which we sure its a valid value (Tune this depending on sensor/noise)
#define PULSE_ANALOG_READ_INTERVAL 100 // The analog reading interval which used to prevent spike and noises (Tune this depending on sensor/noise)

class PulseSensor
{
public:
      static unsigned long last_beat_analog_read;
      static unsigned long last_beat_time;
      static int BPM;
      static bool beat_detected;
      static uint16_t beat_analog_samples[PULSE_ANALOG_SCOUNT];
      static uint8_t beat_analog_sample_index;

      static void init();
      static void loop();
      static float get_hr();
};
